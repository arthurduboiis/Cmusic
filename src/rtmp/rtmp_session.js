const QueryString = require("querystring")

const AV = require('../core/core_av');
const { AUDIO_SOUND_RATE, AUDIO_CODEC_NAME} = require('../core/core_av');

const Handshake = require("./rtmp_handshake")
const context = require("../core/core_ctx")
const AMF = require("../core/core_amf")
const CoreUtils = require("../core/core_utils");

const RTMP_HANDSHAKE_SIZE = 1536
const RTMP_HANDSHAKE_UNINIT = 0
const RTMP_HANDSHAKE_0 = 1
const RTMP_HANDSHAKE_1 = 2
const RTMP_HANDSHAKE_2 = 3

const RTMP_PARSE_INIT = 0
const RTMP_PARSE_BASIC_HEADER = 1
const RTMP_PARSE_MESSAGE_HEADER = 2
const RTMP_PARSE_EXTENDED_TIMESTAMP = 3
const RTMP_PARSE_PAYLOAD = 4

const MAX_CHUNK_HEADER = 18

const RTMP_CHUNK_TYPE_0 = 0 // 11-bytes: timestamp(3) + length(3) + stream type(1) + stream id(4)
const RTMP_CHUNK_TYPE_1 = 1 // 7-bytes: delta(3) + length(3) + stream type(1)
const RTMP_CHUNK_TYPE_2 = 2 // 3-bytes: delta(3)
const RTMP_CHUNK_TYPE_3 = 3 // 0-byte

const RTMP_CHANNEL_PROTOCOL = 2
const RTMP_CHANNEL_INVOKE = 3
const RTMP_CHANNEL_AUDIO = 4
const RTMP_CHANNEL_DATA = 6;

const rtmpHeaderSize = [11, 7, 3, 0]

/* Protocol Control Messages */
const RTMP_TYPE_SET_CHUNK_SIZE = 1
const RTMP_TYPE_ABORT = 2
const RTMP_TYPE_ACKNOWLEDGEMENT = 3 // bytes read report
const RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE = 5 // server bandwidth
const RTMP_TYPE_SET_PEER_BANDWIDTH = 6 // client bandwidth

/* User Control Messages Event (4) */
const RTMP_TYPE_EVENT = 4
const RTMP_TYPE_AUDIO = 8

const RTMP_TYPE_DATA = 18; // AMF0
const RTMP_TYPE_FLEX_MESSAGE = 17; // AMF3
const RTMP_TYPE_INVOKE = 20; // AMF0
const RTMP_TYPE_METADATA = 22;

const RTMP_CHUNK_SIZE = 128
const RTMP_PING_TIME = 60000
const RTMP_PING_TIMEOUT = 30000

const STREAM_BEGIN = 0x00
const STREAM_EOF = 0x01

const RtmpPacket = {
    create: (fmt = 0, cid = 0) => {
        return {
            header: {
                fmt: fmt,
                cid: cid,
                timestamp: 0,
                length: 0,
                type: 0,
                stream_id: 0
            },
            clock: 0,
            payload: null,
            capacity: 0,
            bytes: 0
        }
    }
}

class rtmp_session{
    constructor(config, socket){
        this.config = config
        this.socket = socket
        this.res = socket
        this.id = CoreUtils.generateNewSessionID()
        this.tag = 'rtmp'

        this.handshakePayload = Buffer.alloc(RTMP_HANDSHAKE_SIZE)
        this.handshakeState = RTMP_HANDSHAKE_UNINIT
        this.handshakeBytes = 0

        this.parserBuffer = Buffer.alloc(MAX_CHUNK_HEADER)
        this.parserState = RTMP_PARSE_INIT
        this.parserBytes = 0
        this.parserBasicBytes = 0
        this.parserPacket = null
        this.inPackets = new Map()
        this.inChunkSize = RTMP_CHUNK_SIZE
        this.outChunkSize = config.rtmp.chunk_size ? config.rtmp.chunk_size : RTMP_CHUNK_SIZE
        this.pingTime = config.rtmp.ping ? config.rtmp.ping * 1000 : RTMP_PING_TIME
        this.pingTimeout = config.rtmp.ping_timeout ? config.rtmp.ping_timeout * 1000 : RTMP_PING_TIMEOUT
        this.pingInterval = null

        this.isStarting = false
        this.isPublishing = false
        this.isPlaying = false
        this.isIdling = false
        this.isPause = false
        this.isReceiveAudio = true
        this.metaData = null
        this.aacSequenceHeader = null
        this.audioCodec = 0

        this.ackSize = 0
        this.inAckSize = 0
        this.inLastAck = 0

        this.appname = ''
        this.streams = 0

        this.playStreamId = 0
        this.playStreamPath = ''
        this.playArgs = {}

        this.publishStreamId = 0
        this.publishStreamPath = ''
        this.publishArgs = {}

        this.players = new Set()
        this.numPlayCache = 0
        this.bitrateCache = {}
        context.sessions.set(this.id, this)
    }

    run(){
        this.socket.on('data', this.onSocketData.bind(this))
        this.socket.on('close', this.onSocketClose.bind(this))
        this.socket.on('error', this.onSocketError.bind(this))
        this.socket.on('timeout', this.onSocketTimeout.bind(this))
        this.socket.setTimeout(this.pingTimeout)
        this.isStarting = true
    }

    stop(){
        if (this.isStarting) {
            this.isStarting = false

            if (this.playStreamId > 0) {
                this.onDeleteStream({streamId: this.playStreamId})
            }

            if (this.publishStreamId > 0) {
                this.onDeleteStream({streamId: this.publishStreamId})
            }
            if (this.pingInterval != null) {
                clearInterval(this.pingInterval)
                this.pingInterval = null
            }
            console.log("[RTMP] rtmp disconnect " + this.id)
            context.nodeEvent.emit('doneConnect', this.id, this.connectCmdObj)

            context.sessions.delete(this.id)
            this.socket.destroy()
        }
    }

    reject(){
        console.log("[RTMP] rtmp reject" + this.id)
        this.stop()
    }
    flush(){
        if(this.numPlayCache > 0){
            console.log("[RTMP] rtmp flush" + this.id)
            this.res.uncork()
        }
    }
    onSocketClose(){
        console.log("[RTMP] rtmp close" + this.id)
        this.stop()
    }
    onSocketError(){
        console.log("[RTMP] rtmp error" + this.id)
        this.stop()
    }
    onSocketTimeout(){
        console.log("rtmp timeout" + this.id)
        this.stop()
    }

    onSocketData(data) {
        let bytes = data.length
        let p = 0
        let n = 0
        while (bytes > 0) {
            switch (this.handshakeState) {
                case RTMP_HANDSHAKE_UNINIT:
                    this.handshakeState = RTMP_HANDSHAKE_0
                    this.handshakeBytes = 0
                    bytes -= 1
                    p += 1
                    break
                case RTMP_HANDSHAKE_0:
                    n = RTMP_HANDSHAKE_SIZE - this.handshakeBytes
                    n = n <= bytes ? n : bytes
                    data.copy(this.handshakePayload, this.handshakeBytes, p, p + n)
                    this.handshakeBytes += n
                    bytes -= n
                    p += n
                    if (this.handshakeBytes === RTMP_HANDSHAKE_SIZE) {
                        this.handshakeState = RTMP_HANDSHAKE_1
                        this.handshakeBytes = 0
                        let s0s1s2 = Handshake.generateS0S1S2(this.handshakePayload)
                        this.socket.write(s0s1s2)
                    }
                    break
                case RTMP_HANDSHAKE_1:
                    n = RTMP_HANDSHAKE_SIZE - this.handshakeBytes
                    n = n <= bytes ? n : bytes
                    data.copy(this.handshakePayload, this.handshakeBytes, p, n)
                    this.handshakeBytes += n
                    bytes -= n
                    p += n
                    if (this.handshakeBytes === RTMP_HANDSHAKE_SIZE) {
                        this.handshakeState = RTMP_HANDSHAKE_2
                        this.handshakeBytes = 0
                        this.handshakePayload = null
                    }
                    break
                case RTMP_HANDSHAKE_2:
                default:
                    return this.rtmpChunkRead(data, p, bytes)
            }
        }
    }

    rtmpChunkBasicHeaderCreate(fmt, cid) {
        let out
        if (cid >= 64 + 255) {
            out = Buffer.alloc(3)
            out[0] = (fmt << 6) | 1
            out[1] = (cid - 64) & 0xff
            out[2] = ((cid - 64) >> 8) & 0xff
        } else if (cid >= 64) {
            out = Buffer.alloc(2)
            out[0] = (fmt << 6) | 0
            out[1] = (cid - 64) & 0xff
        } else {
            out = Buffer.alloc(1)
            out[0] = (fmt << 6) | cid
        }
        return out
    }

    rtmpChunkMessageHeaderCreate(header) {
        let out = Buffer.alloc(rtmpHeaderSize[header.fmt % 4])
        if (header.fmt <= RTMP_CHUNK_TYPE_2) {
            out.writeUIntBE(header.timestamp >= 0xffffff ? 0xffffff : header.timestamp, 0, 3)
        }

        if (header.fmt <= RTMP_CHUNK_TYPE_1) {
            out.writeUIntBE(header.length, 3, 3)
            out.writeUInt8(header.type, 6)
        }

        if (header.fmt === RTMP_CHUNK_TYPE_0) {
            out.writeUInt32LE(header.stream_id, 7)
        }
        return out
    }

    rtmpChunksCreate(packet) {
        let header = packet.header
        let payload = packet.payload
        let payloadSize = header.length
        let chunkSize = this.outChunkSize
        let chunksOffset = 0
        let payloadOffset = 0
        let chunkBasicHeader = this.rtmpChunkBasicHeaderCreate(header.fmt, header.cid)
        let chunkBasicHeader3 = this.rtmpChunkBasicHeaderCreate(RTMP_CHUNK_TYPE_3, header.cid)
        let chunkMessageHeader = this.rtmpChunkMessageHeaderCreate(header)
        let useExtendedTimestamp = header.timestamp >= 0xffffff
        let headerSize = chunkBasicHeader.length + chunkMessageHeader.length + (useExtendedTimestamp ? 4 : 0)
        let n = headerSize + payloadSize + Math.floor(payloadSize / chunkSize)

        if (useExtendedTimestamp) {
            n += Math.floor(payloadSize / chunkSize) * 4
        }
        if (!(payloadSize % chunkSize)) {
            n -= 1
            if (useExtendedTimestamp) {
                n -= 4
            }
        }
        let chunks = Buffer.alloc(n)
        chunkBasicHeader.copy(chunks, chunksOffset)
        chunksOffset += chunkBasicHeader.length
        chunkMessageHeader.copy(chunks, chunksOffset)
        chunksOffset += chunkMessageHeader.length
        if (useExtendedTimestamp) {
            chunks.writeUInt32BE(header.timestamp, chunksOffset)
            chunksOffset += 4
        }
        while (payloadSize > 0) {
            if (payloadSize > chunkSize) {
                payload.copy(chunks, chunksOffset, payloadOffset, payloadOffset + chunkSize)
                payloadSize -= chunkSize
                chunksOffset += chunkSize
                payloadOffset += chunkSize
                chunkBasicHeader3.copy(chunks, chunksOffset)
                chunksOffset += chunkBasicHeader3.length
                if (useExtendedTimestamp) {
                    chunks.writeUInt32BE(header.timestamp, chunksOffset)
                    chunksOffset += 4
                }
            } else {
                payload.copy(chunks, chunksOffset, payloadOffset, payloadOffset + payloadSize)
                payloadSize -= payloadSize
                chunksOffset += payloadSize
                payloadOffset += payloadSize
            }
        }
        return chunks
    }

    rtmpChunkRead(data, p, bytes) {
        let size = 0
        let offset = 0
        let extended_timestamp = 0

        while (offset < bytes) {
            switch (this.parserState) {
                case RTMP_PARSE_INIT:
                    this.parserBytes = 1
                    this.parserBuffer[0] = data[p + offset++]
                    if (0 === (this.parserBuffer[0] & 0x3f)) {
                        this.parserBasicBytes = 2
                    } else if (1 === (this.parserBuffer[0] & 0x3f)) {
                        this.parserBasicBytes = 3
                    } else {
                        this.parserBasicBytes = 1
                    }
                    this.parserState = RTMP_PARSE_BASIC_HEADER
                    break
                case RTMP_PARSE_BASIC_HEADER:
                    while (this.parserBytes < this.parserBasicBytes && offset < bytes) {
                        this.parserBuffer[this.parserBytes++] = data[p + offset++]
                    }
                    if (this.parserBytes >= this.parserBasicBytes) {
                        this.parserState = RTMP_PARSE_MESSAGE_HEADER
                    }
                    break
                case RTMP_PARSE_MESSAGE_HEADER:
                    size = rtmpHeaderSize[this.parserBuffer[0] >> 6] + this.parserBasicBytes
                    while (this.parserBytes < size && offset < bytes) {
                        this.parserBuffer[this.parserBytes++] = data[p + offset++]
                    }
                    if (this.parserBytes >= size) {
                        this.rtmpPacketParse()
                        this.parserState = RTMP_PARSE_EXTENDED_TIMESTAMP
                    }
                    break
                case RTMP_PARSE_EXTENDED_TIMESTAMP:
                    size = rtmpHeaderSize[this.parserPacket.header.fmt] + this.parserBasicBytes
                    if (this.parserPacket.header.timestamp === 0xffffff) size += 4
                    while (this.parserBytes < size && offset < bytes) {
                        this.parserBuffer[this.parserBytes++] = data[p + offset++]
                    }
                    if (this.parserBytes >= size) {
                        if (this.parserPacket.header.timestamp === 0xffffff) {
                            extended_timestamp = this.parserBuffer.readUInt32BE(rtmpHeaderSize[this.parserPacket.header.fmt] + this.parserBasicBytes)
                        } else {
                            extended_timestamp = this.parserPacket.header.timestamp
                        }

                        if (this.parserPacket.bytes === 0) {
                            if (RTMP_CHUNK_TYPE_0 === this.parserPacket.header.fmt) {
                                this.parserPacket.clock = extended_timestamp
                            } else {
                                this.parserPacket.clock += extended_timestamp
                            }
                            this.rtmpPacketAlloc()
                        }
                        this.parserState = RTMP_PARSE_PAYLOAD
                    }
                    break
                case RTMP_PARSE_PAYLOAD:
                    size = Math.min(this.inChunkSize - (this.parserPacket.bytes % this.inChunkSize), this.parserPacket.header.length - this.parserPacket.bytes)
                    size = Math.min(size, bytes - offset)
                    if (size > 0) {
                        data.copy(this.parserPacket.payload, this.parserPacket.bytes, p + offset, p + offset + size)
                    }
                    this.parserPacket.bytes += size
                    offset += size

                    if (this.parserPacket.bytes >= this.parserPacket.header.length) {
                        this.parserState = RTMP_PARSE_INIT
                        this.parserPacket.bytes = 0
                        if (this.parserPacket.clock > 0xffffffff) {
                            break
                        }
                        this.rtmpHandler()
                    } else if (0 === this.parserPacket.bytes % this.inChunkSize) {
                        this.parserState = RTMP_PARSE_INIT
                    }
                    break
            }
        }

        this.inAckSize += data.length
        if (this.inAckSize >= 0xf0000000) {
            this.inAckSize = 0
            this.inLastAck = 0
        }
        if (this.ackSize > 0 && this.inAckSize - this.inLastAck >= this.ackSize) {
            this.inLastAck = this.inAckSize
            this.sendACK(this.inAckSize)
        }

        this.bitrateCache.bytes += bytes
        let current_time = Date.now()
        let diff = current_time - this.bitrateCache.last_update
        if (diff >= this.bitrateCache.intervalMs) {
            this.bitrate = Math.round(this.bitrateCache.bytes * 8 / diff)
            this.bitrateCache.bytes = 0
            this.bitrateCache.last_update = current_time
        }
    }

    rtmpPacketParse() {
        let fmt = this.parserBuffer[0] >> 6
        let cid = 0
        if (this.parserBasicBytes === 2) {
            cid = 64 + this.parserBuffer[1]
        } else if (this.parserBasicBytes === 3) {
            cid = (64 + this.parserBuffer[1] + this.parserBuffer[2]) << 8
        } else {
            cid = this.parserBuffer[0] & 0x3f
        }
        let hasp = this.inPackets.has(cid)
        if (!hasp) {
            this.parserPacket = RtmpPacket.create(fmt, cid)
            this.inPackets.set(cid, this.parserPacket)
        } else {
            this.parserPacket = this.inPackets.get(cid)
        }
        this.parserPacket.header.fmt = fmt
        this.parserPacket.header.cid = cid
        this.rtmpChunkMessageHeaderRead()

        if (this.parserPacket.header.type > RTMP_TYPE_METADATA) {
            console.log("[RTMP] rtmp packet parse error")
            this.stop()
        }
    }

    rtmpChunkMessageHeaderRead() {
        let offset = this.parserBasicBytes

        // timestamp / delta
        if (this.parserPacket.header.fmt <= RTMP_CHUNK_TYPE_2) {
            this.parserPacket.header.timestamp = this.parserBuffer.readUIntBE(offset, 3)
            offset += 3
        }

        // message length + type
        if (this.parserPacket.header.fmt <= RTMP_CHUNK_TYPE_1) {
            this.parserPacket.header.length = this.parserBuffer.readUIntBE(offset, 3)
            this.parserPacket.header.type = this.parserBuffer[offset + 3]
            offset += 4
        }

        if (this.parserPacket.header.fmt === RTMP_CHUNK_TYPE_0) {
            this.parserPacket.header.stream_id = this.parserBuffer.readUInt32LE(offset)
            offset += 4
        }
        return offset
    }

    rtmpPacketAlloc() {
        if (this.parserPacket.capacity < this.parserPacket.header.length) {
            this.parserPacket.payload = Buffer.alloc(this.parserPacket.header.length + 1024)
            this.parserPacket.capacity = this.parserPacket.header.length + 1024
        }
    }

    rtmpHandler() {
        switch (this.parserPacket.header.type) {
            case RTMP_TYPE_SET_CHUNK_SIZE:
            case RTMP_TYPE_ABORT:
            case RTMP_TYPE_ACKNOWLEDGEMENT:
            case RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE:
            case RTMP_TYPE_SET_PEER_BANDWIDTH:
                return 0 === this.rtmpControlHandler() ? -1 : 0
            case RTMP_TYPE_AUDIO:
                return this.rtmpAudioHandler()
            case RTMP_TYPE_INVOKE:
                return this.rtmpInvokeHandler()
        }
    }
    rtmpControlHandler() {
        let payload = this.parserPacket.payload
        switch (this.parserPacket.header.type) {
            case RTMP_TYPE_SET_CHUNK_SIZE:
                this.inChunkSize = payload.readUInt32BE()
                break
            case RTMP_TYPE_ABORT:
                break
            case RTMP_TYPE_ACKNOWLEDGEMENT:
                break
            case RTMP_TYPE_WINDOW_ACKNOWLEDGEMENT_SIZE:
                this.ackSize = payload.readUInt32BE()
                break
            case RTMP_TYPE_SET_PEER_BANDWIDTH:
                break
        }
    }

    rtmpAudioHandler() {
        let payload = this.parserPacket.payload.slice(0, this.parserPacket.header.length)
        let sound_format = (payload[0] >> 4) & 0x0f
        let sound_type = payload[0] & 0x01
        let sound_size = (payload[0] >> 1) & 0x01
        let sound_rate = (payload[0] >> 2) & 0x03

        if (this.audioCodec == 0) {
            this.audioCodec = sound_format
            this.audioCodecName = AUDIO_CODEC_NAME[sound_format]
            this.audioSamplerate = AUDIO_SOUND_RATE[sound_rate]
            this.audioChannels = ++sound_type

            if (sound_format == 4) {
                this.audioSamplerate = 16000
            } else if (sound_format == 5) {
                this.audioSamplerate = 8000
            } else if (sound_format == 11) {
                this.audioSamplerate = 16000
            } else if (sound_format == 14) {
                this.audioSamplerate = 8000
            }

            if (sound_format != 10 && sound_format != 13) {
                    console.log("[RTMP] sound format error")
            }
        }

        if ((sound_format == 10 || sound_format == 13) && payload[1] == 0) {
            //cache aac sequence header
            this.isFirstAudioReceived = true
            this.aacSequenceHeader = Buffer.alloc(payload.length)
            payload.copy(this.aacSequenceHeader)
            if(sound_format == 10) {
                let info = AV.readAACSpecificConfig(this.aacSequenceHeader)
                this.audioProfileName = AV.getAACProfileName(info)
                this.audioSamplerate = info.sample_rate
                this.audioChannels = info.channels
            } else {
                this.audioSamplerate = 48000
                this.audioChannels = payload[11]
            }
            console.log("[RTMP] don't know weird things with sound format \_(ツ)_/")

        }

        let packet = RtmpPacket.create()
        packet.header.fmt = RTMP_CHUNK_TYPE_0
        packet.header.cid = RTMP_CHANNEL_AUDIO
        packet.header.type = RTMP_TYPE_AUDIO
        packet.payload = payload
        packet.header.length = packet.payload.length
        packet.header.timestamp = this.parserPacket.clock
        let rtmpChunks = this.rtmpChunksCreate(packet)

        for (let playerId of this.players) {
            let playerSession = context.sessions.get(playerId)

            if (playerSession.numPlayCache === 0) {
                playerSession.res.cork()
            }

            if (playerSession instanceof rtmp_session) {
                if (playerSession.isStarting && playerSession.isPlaying && !playerSession.isPause && playerSession.isReceiveAudio) {
                    rtmpChunks.writeUInt32LE(playerSession.playStreamId, 8)
                    playerSession.res.write(rtmpChunks)
                }
            }

            playerSession.numPlayCache++

            if (playerSession.numPlayCache === 10) {
                process.nextTick(() => playerSession.res.uncork())
                playerSession.numPlayCache = 0
            }
        }
    }

    rtmpInvokeHandler() {
        let offset = this.parserPacket.header.type === RTMP_TYPE_FLEX_MESSAGE ? 1 : 0
        let payload = this.parserPacket.payload.slice(offset, this.parserPacket.header.length)
        let invokeMessage = AMF.decodeAmf0Cmd(payload)
        switch (invokeMessage.cmd) {
            case 'connect':
                this.onConnect(invokeMessage)
                break
            case 'releaseStream':
                break
            case 'FCPublish':
                break
            case 'createStream':
                this.onCreateStream(invokeMessage)
                break
            case 'publish':
                this.onPublish(invokeMessage)
                break
            case 'play':
                this.onPlay(invokeMessage)
                break
            case 'FCUnpublish':
                break
            case 'deleteStream':
                this.onDeleteStream(invokeMessage)
                break
            case 'closeStream':
                this.onCloseStream()
                break
            case 'receiveAudio':
                this.onReceiveAudio(invokeMessage)
                break
            default:
                break
        }
    }

    sendACK(size) {
        let rtmpBuffer = Buffer.from('02000000000004030000000000000000', 'hex')
        rtmpBuffer.writeUInt32BE(size, 12)
        this.socket.write(rtmpBuffer)
    }

    sendWindowACK(size) {
        let rtmpBuffer = Buffer.from('02000000000004050000000000000000', 'hex')
        rtmpBuffer.writeUInt32BE(size, 12)
        this.socket.write(rtmpBuffer)
    }

    setPeerBandwidth(size, type) {
        let rtmpBuffer = Buffer.from('0200000000000506000000000000000000', 'hex')
        rtmpBuffer.writeUInt32BE(size, 12)
        rtmpBuffer[16] = type
        this.socket.write(rtmpBuffer)
    }

    setChunkSize(size) {
        let rtmpBuffer = Buffer.from('02000000000004010000000000000000', 'hex')
        rtmpBuffer.writeUInt32BE(size, 12)
        this.socket.write(rtmpBuffer)
    }

    sendStreamStatus(st, id) {
        let rtmpBuffer = Buffer.from('020000000000060400000000000000000000', 'hex')
        rtmpBuffer.writeUInt16BE(st, 12)
        rtmpBuffer.writeUInt32BE(id, 14)
        this.socket.write(rtmpBuffer)
    }

    sendInvokeMessage(sid, opt) {
        let packet = RtmpPacket.create()
        packet.header.fmt = RTMP_CHUNK_TYPE_0
        packet.header.cid = RTMP_CHANNEL_INVOKE
        packet.header.type = RTMP_TYPE_INVOKE
        packet.header.stream_id = sid
        packet.payload = AMF.encodeAmf0Cmd(opt)
        packet.header.length = packet.payload.length
        let chunks = this.rtmpChunksCreate(packet)
        this.socket.write(chunks)
    }
    sendDataMessage(opt, sid) {
        let packet = RtmpPacket.create()
        packet.header.fmt = RTMP_CHUNK_TYPE_0
        packet.header.cid = RTMP_CHANNEL_DATA
        packet.header.type = RTMP_TYPE_DATA
        packet.payload = AMF.encodeAmf0Data(opt)
        packet.header.length = packet.payload.length
        packet.header.stream_id = sid
        let chunks = this.rtmpChunksCreate(packet)
        this.socket.write(chunks)
    }

    sendStatusMessage(sid, level, code, description) {
        let opt = {
            cmd: 'onStatus',
            transId: 0,
            cmdObj: null,
            info: {
                level: level,
                code: code,
                description: description
            }
        }
        this.sendInvokeMessage(sid, opt)
    }

    sendRtmpSampleAccess(sid) {
        let opt = {
            cmd: '|RtmpSampleAccess',
            bool1: false,
            bool2: false
        }
        this.sendDataMessage(opt, sid)
    }

    sendPingRequest() {
        let currentTimestamp = Date.now() - this.startTimestamp
        let packet = RtmpPacket.create()
        packet.header.fmt = RTMP_CHUNK_TYPE_0
        packet.header.cid = RTMP_CHANNEL_PROTOCOL
        packet.header.type = RTMP_TYPE_EVENT
        packet.header.timestamp = currentTimestamp
        packet.payload = Buffer.from([0, 6, (currentTimestamp >> 24) & 0xff, (currentTimestamp >> 16) & 0xff, (currentTimestamp >> 8) & 0xff, currentTimestamp & 0xff])
        packet.header.length = packet.payload.length
        let chunks = this.rtmpChunksCreate(packet)
        this.socket.write(chunks)
    }

    respondConnect(tid) {
        let opt = {
            cmd: '_result',
            transId: tid,
            cmdObj: {
                fmsVer: 'FMS/3,0,1,123',
                capabilities: 31
            },
            info: {
                level: 'status',
                code: 'NetConnection.Connect.Success',
                description: 'Connection succeeded.',
                objectEncoding: this.objectEncoding
            }
        }
        this.sendInvokeMessage(0, opt)
    }
    respondCreateStream(tid) {
        this.streams++
        let opt = {
            cmd: '_result',
            transId: tid,
            cmdObj: null,
            info: this.streams
        }
        this.sendInvokeMessage(0, opt)
    }

    respondPlay() {
        this.sendStreamStatus(STREAM_BEGIN, this.playStreamId)
        this.sendStatusMessage(this.playStreamId, 'status', 'NetStream.Play.Reset', 'Playing and resetting stream.')
        this.sendStatusMessage(this.playStreamId, 'status', 'NetStream.Play.Start', 'Started playing stream.')
        this.sendRtmpSampleAccess()
    }

    onConnect(invokeMessage) {
        invokeMessage.cmdObj.app = invokeMessage.cmdObj.app.replace('/', '') //fix jwplayer
        context.nodeEvent.emit('preConnect', this.id, invokeMessage.cmdObj)
        if (!this.isStarting) {
            return
        }
        this.connectCmdObj = invokeMessage.cmdObj
        this.appname = invokeMessage.cmdObj.app
        this.objectEncoding = invokeMessage.cmdObj.objectEncoding != null ? invokeMessage.cmdObj.objectEncoding : 0
        this.connectTime = new Date()
        this.startTimestamp = Date.now()
        this.pingInterval = setInterval(() => {
            this.sendPingRequest()
        }, this.pingTime)
        this.sendWindowACK(5000000)
        this.setPeerBandwidth(5000000, 2)
        this.setChunkSize(this.outChunkSize)
        this.respondConnect(invokeMessage.transId)
        this.bitrateCache = {
            intervalMs: 1000,
            last_update: this.startTimestamp,
            bytes: 0,
        }
        console.log("[RTMP] rtmp connect " + this.id + " " + this.appname)
        context.nodeEvent.emit('postConnect', this.id, invokeMessage.cmdObj)
    }

    onCreateStream(invokeMessage) {
        this.respondCreateStream(invokeMessage.transId)
    }
    onPublish(invokeMessage) {
        if (typeof invokeMessage.streamName !== 'string') {
            return
        }
        this.publishStreamPath = '/' + this.appname + '/' + invokeMessage.streamName.split('?')[0]
        this.publishArgs = QueryString.parse(invokeMessage.streamName.split('?')[1])
        this.publishStreamId = this.parserPacket.header.stream_id
        context.nodeEvent.emit('prePublish', this.id, this.publishStreamPath, this.publishArgs)
        if (!this.isStarting) {
            return
        }

        if (context.publishers.has(this.publishStreamPath)) {
            this.reject()
            console.log("[RTMP] rtmp already has a stream" + this.id)
            this.sendStatusMessage(this.publishStreamId, 'error', 'NetStream.Publish.BadName', 'Stream already publishing')
        } else if (this.isPublishing) {
            console.log("[RTMP] rtmp NetConnection is publishing" + this.id)
            this.sendStatusMessage(this.publishStreamId, 'error', 'NetStream.Publish.BadConnection', 'Connection already publishing')
        } else {
            console.log("[RTMP] rtmp New Stream " + this.id)
            context.publishers.set(this.publishStreamPath, this.id)
            this.isPublishing = true

            this.sendStatusMessage(this.publishStreamId, 'status', 'NetStream.Publish.Start', `${this.publishStreamPath} is now published.`)
            for (let idlePlayerId of context.idlePlayers) {
                let idlePlayer = context.sessions.get(idlePlayerId)
                if (idlePlayer && idlePlayer.playStreamPath === this.publishStreamPath) {
                    idlePlayer.onStartPlay()
                    context.idlePlayers.delete(idlePlayerId)
                }
            }
            context.nodeEvent.emit('postPublish', this.id, this.publishStreamPath, this.publishArgs)
        }
    }

    onPlay(invokeMessage) {
        if (typeof invokeMessage.streamName !== 'string') {
            return
        }
        this.playStreamPath = '/' + this.appname + '/' + invokeMessage.streamName.split('?')[0]
        this.playArgs = QueryString.parse(invokeMessage.streamName.split('?')[1])
        this.playStreamId = this.parserPacket.header.stream_id
        context.nodeEvent.emit('prePlay', this.id, this.playStreamPath, this.playArgs)

        if (!this.isStarting) {
            return
        }

        if (this.isPlaying) {
            console.log("[RTMP] rtmp NetConnection is playing" + this.id)
            this.sendStatusMessage(this.playStreamId, 'error', 'NetStream.Play.BadConnection', 'Connection already playing')
        } else {
            this.respondPlay()
        }

        if (context.publishers.has(this.playStreamPath)) {
            this.onStartPlay()
        } else {
            console.log("[RTMP] rtmp Stream not found" + this.id)
            this.isIdling = true
            context.idlePlayers.add(this.id)
        }
    }

    onStartPlay() {
        let publisherId = context.publishers.get(this.playStreamPath)
        let publisher = context.sessions.get(publisherId)
        let players = publisher.players
        players.add(this.id)

        if (publisher.metaData != null) {
            let packet = RtmpPacket.create()
            packet.header.fmt = RTMP_CHUNK_TYPE_0
            packet.header.cid = RTMP_CHANNEL_DATA
            packet.header.type = RTMP_TYPE_DATA
            packet.payload = publisher.metaData
            packet.header.length = packet.payload.length
            packet.header.stream_id = this.playStreamId
            let chunks = this.rtmpChunksCreate(packet)
            this.socket.write(chunks)
        }

        if (publisher.audioCodec === 10 || publisher.audioCodec === 13) {
            let packet = RtmpPacket.create()
            packet.header.fmt = RTMP_CHUNK_TYPE_0
            packet.header.cid = RTMP_CHANNEL_AUDIO
            packet.header.type = RTMP_TYPE_AUDIO
            packet.payload = publisher.aacSequenceHeader
            packet.header.length = packet.payload.length
            packet.header.stream_id = this.playStreamId
            let chunks = this.rtmpChunksCreate(packet)
            this.socket.write(chunks)
        }

        this.isIdling = false
        this.isPlaying = true
        context.nodeEvent.emit('postPlay', this.id, this.playStreamPath, this.playArgs)
        console.log("[RTMP] rtmp Join stream " + this.id)
    }
    onReceiveAudio(invokeMessage) {
        this.isReceiveAudio = invokeMessage.bool
        console.log("[RTMP] rtmp play receiveAUdio" + this.id)
    }

    onCloseStream() {
        //red5-publisher
        let closeStream = { streamId: this.parserPacket.header.stream_id }
        this.onDeleteStream(closeStream)
    }

    onDeleteStream(invokeMessage) {
        if (invokeMessage.streamId == this.playStreamId) {
            if (this.isIdling) {
                context.idlePlayers.delete(this.id)
                this.isIdling = false
            } else {
                let publisherId = context.publishers.get(this.playStreamPath)
                if (publisherId != null) {
                    context.sessions.get(publisherId).players.delete(this.id)
                }
                context.nodeEvent.emit('donePlay', this.id, this.playStreamPath, this.playArgs)
                this.isPlaying = false
            }
            console.log("[RTMP] rtmp play Close Stream" +this.id)
            if (this.isStarting) {
                this.sendStatusMessage(this.playStreamId, 'status', 'NetStream.Play.Stop', 'Stopped playing stream.')
            }
            this.playStreamId = 0
            this.playStreamPath = ''
        }

        if (invokeMessage.streamId == this.publishStreamId) {

            if (this.isPublishing) {
                console.log("[RTMP] rtmp publish Close Stream" + this.id)
                context.nodeEvent.emit('donePublish', this.id, this.publishStreamPath, this.publishArgs)
                if (this.isStarting) {
                    this.sendStatusMessage(this.publishStreamId, 'status', 'NetStream.Unpublish.Success', `${this.publishStreamPath} is now unpublished.`)
                }

                for (let playerId of this.players) {
                    let playerSession = context.sessions.get(playerId)
                    if (playerSession instanceof rtmp_session) {
                        playerSession.sendStatusMessage(playerSession.playStreamId, 'status', 'NetStream.Play.UnpublishNotify', 'stream is now unpublished.')
                        playerSession.flush()
                    } else {
                        playerSession.stop()
                    }
                }
                for (let playerId of this.players) {
                    let playerSession = context.sessions.get(playerId)
                    context.idlePlayers.add(playerId)
                    playerSession.isPlaying = false
                    playerSession.isIdling = true
                    if (playerSession instanceof rtmp_session) {
                        playerSession.sendStreamStatus(STREAM_EOF, playerSession.playStreamId)
                    }
                }

                context.publishers.delete(this.publishStreamPath)
                this.players.clear()
                this.isPublishing = false
            }
            this.publishStreamId = 0
            this.publishStreamPath = ''
        }
    }
}

module.exports = rtmp_session
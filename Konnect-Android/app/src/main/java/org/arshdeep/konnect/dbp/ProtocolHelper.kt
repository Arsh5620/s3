package org.arshdeep.konnect.dbp

import android.util.Log
import org.arshdeep.konnect.dbp.Exceptions.CorruptedConnectionException
import java.io.BufferedInputStream
import java.io.InputStream
import java.io.OutputStream
import java.io.PrintWriter
import java.lang.Exception
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.charset.Charset

class ProtocolHelper
{
	companion object
	{
		private val LOG_TAG = this::class.java.toString()
		private const val DBP_MAGIC_INDEX = 7
		private const val DBP_HEADER_LENGTH_INDEX = 6
		const val DBP_ALIGN_BYTES = 16
		
		fun writeSimplePacket(param: RequestPOJO, outStream: OutputStream)
		{
			var headers = ProtocolHeader(param.protocolRequestType)
			var header = headers.construct()
			
			var headerAlignLength = header.length / DBP_ALIGN_BYTES
			var dataLength = param.data?.length!!.toLong()
			
			var byteArray = getHeader8(dataLength, headerAlignLength)
			
			byteArray += header.toByteArray(Charset.defaultCharset())
			byteArray += param.data!!.toByteArray(Charset.defaultCharset())
			
			outStream.write(byteArray)
		}
		
		fun readResponsePacket(inStream: InputStream): ResponsePOJO
		{
			var response = ResponsePOJO()
			var bReader = BufferedInputStream(inStream)
			var header8Bytes = ByteArray(8)
			var read = bReader.read(header8Bytes)
			
			if (read != header8Bytes.count())
			{
				throw Exception("Read network IO error")
			}
			
			if (header8Bytes[DBP_MAGIC_INDEX] == 0xD0.toByte())
			{
				var headerSize =
					header8Bytes[DBP_HEADER_LENGTH_INDEX].toUByte().toInt() * DBP_ALIGN_BYTES
				
				var keyValuePairHeader = ByteArray(headerSize)
				bReader.read(keyValuePairHeader)
				
				header8Bytes[6] = 0
				header8Bytes[7] = 0
				var dataSize =
					ByteBuffer.wrap(header8Bytes, 0, 8).order(ByteOrder.LITTLE_ENDIAN).long
				
				var actualData = ByteArray(dataSize.toInt())
				bReader.read(actualData)
				
				var string = String(actualData)
				response.data	= string
			}
			else
			{
				throw CorruptedConnectionException()
			}
			return (response)
		}
		
		private fun getHeader8(dataSize: Long, headerSize: Int): ByteArray
		{
			var byteBuffer: ByteBuffer = ByteBuffer.allocate(8)
			var h1 = 0xD0000000
			var h2 = 0x00000000
			
			h2 = h2 or ((dataSize and 0xFFFFFFFF).toInt())
			h1 = h1 or ((dataSize shr 32) and 0xFFFF)
			h1 = h1 or (((headerSize and 0xFF) shl 16).toLong())
			var m5 = h1.toInt()
			
			byteBuffer.put(
				ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(h2).array(),
				0,
				4
			)
			byteBuffer.put(
				ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(m5).array(),
				0,
				4
			)
			return (byteBuffer.array())
		}
		
		fun padString(header: String, align: Int): String
		{
			return String.format("%-" + (align) + "s", header)
		}
	}
}
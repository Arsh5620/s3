package org.arshdeep.konnect.dbp

import android.os.AsyncTask
import android.util.Log
import org.arshdeep.konnect.dbp.Exceptions.SocketNotInitException
import java.io.BufferedInputStream
import java.io.PrintWriter
import java.nio.ByteBuffer
import java.nio.ByteOrder


class ProtocolAsyncTask : AsyncTask<RequestPOJO, Void, ResponsePOJO>()
{
	private val LOG_TAG:String	= this.javaClass.name
	
	override fun doInBackground(vararg params: RequestPOJO?): ResponsePOJO?
	{
		var param: RequestPOJO? = params[0] ?: return null
		
		if (param?.socket == null)
		{
			throw SocketNotInitException()
		}
		
		var socket	= param.socket!!
		
		var outStream	= socket.getOutputStream()
		ProtocolHelper.writeSimplePacket(param, outStream)
		
		// Now we will get a response, and must process response accordingly
		
		var reader = socket.getInputStream()
		var response	= ProtocolHelper.readResponsePacket(reader)
		
		response.request	= param
		return (response)
	}
	
	override fun onPostExecute(result: ResponsePOJO?)
	{
		if (result != null)
		{
			if (result.request?.callbacks != null)
			{
				result.request?.callbacks!!.callbackResponse(result)
			}
		}
	}
	
}
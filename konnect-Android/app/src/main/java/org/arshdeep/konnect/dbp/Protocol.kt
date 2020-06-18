package org.arshdeep.konnect.dbp

import android.util.Log
import java.lang.Exception
import java.net.Socket

class Protocol
{
	private val LOG_TAG	= this::class.java.toString()
	private var request = RequestPOJO()
	private var callback:ProtocolCallback
	private var username:String
	private var password:String
	
	constructor(username:String, password:String, host: String
				, port: Int, callback: ProtocolCallback)
	{
		this.username	= username
		this.password	= password
		this.callback	= callback
		retryConnection(host, port)
	}
	
	fun retryConnection(host: String, port: Int)
	{
		Log.w(LOG_TAG, "This was called.")
		Thread {
			try
			{
				request.socket = Socket(host, port)
				
				request.isInit = true
				callback.callbackRequest(this.request)
			}
			catch (e: Exception)
			{
				request.isInit = false
				callback.callbackRequest(this.request)
			}
		}.start()
	}
	
	fun sendNotification(notification: String)
	{
		var task = ProtocolAsyncTask()
		request.data = notification
		request.protocolRequestType	= ProtocolRequestTypeEnum.REQUEST_TYPE_NOTIFICATION
		
		task.execute(request)
	}
	
	fun getServerInfo(protocolCallback: ProtocolCallback)
	{
		var task = ProtocolAsyncTask();
		request.data = ""
		request.protocolRequestType	= ProtocolRequestTypeEnum.REQUEST_TYPE_SERVER
		request.callbacks	= protocolCallback
		task.execute(request)
	}
}
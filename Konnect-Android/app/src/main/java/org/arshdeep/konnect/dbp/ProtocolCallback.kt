package org.arshdeep.konnect.dbp

interface ProtocolCallback
{
	fun callbackRequest(instance: RequestPOJO) {}
	fun callbackResponse(instance: ResponsePOJO){}
}
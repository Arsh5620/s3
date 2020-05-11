package org.arshdeep.konnect.dbp

import java.net.Socket

class RequestPOJO
{
	var socket:Socket?	= null
	var isInit = false
	var protocolRequestType = ProtocolRequestTypeEnum.REQUEST_TYPE_NONE
	var data:String?	= null
	var callbacks:ProtocolCallback?	= null
}
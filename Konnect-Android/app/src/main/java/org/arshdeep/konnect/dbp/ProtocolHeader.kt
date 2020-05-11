package org.arshdeep.konnect.dbp

import java.lang.StringBuilder

class ProtocolHeader
{
	
	public class KeyValuePairs
	{
		private var key: String = ""
		fun getKey(): String
		{
			return this.key
		}
		
		private var value: String = ""
		fun getValue(): String
		{
			return this.value
		}
		
		
		constructor(key: String, value: String)
		{
			this.key = key
			this.value = value
		}
	}
	
	private var actionType = ProtocolRequestTypeEnum.REQUEST_TYPE_NONE
	private var actionAttributes = ArrayList<KeyValuePairs>(10)
	
	constructor(actionType: ProtocolRequestTypeEnum)
	{
		this.actionType = actionType
	}
	
	constructor(actionType: ProtocolRequestTypeEnum, listOfAttributes: ArrayList<KeyValuePairs>)
			: this(actionType)
	{
		this.actionAttributes = listOfAttributes
	}
	
	fun construct(): String
	{
		var builder: StringBuilder = StringBuilder();
		builder.append("action=\"" + getActionTypeString(this.actionType) + "\"\n");
		
		for (item in actionAttributes)
		{
			builder.append(item.getKey() + "=\"" + item.getValue() + "\"\n")
		}
		
		var length = getHeaderAlignment(builder.length)
		
		return ProtocolHelper.padString(builder.toString(), length)
	}
	
	private fun getHeaderAlignment(length: Int): Int
	{
		var result = length
		return if ((length % ProtocolHelper.DBP_ALIGN_BYTES) > 0)
		{
			result += ProtocolHelper.DBP_ALIGN_BYTES - (length % ProtocolHelper.DBP_ALIGN_BYTES)
			result
		}
		else
		{
			result
		}
	}
	
	private fun getActionTypeString(actionType: ProtocolRequestTypeEnum): String
	{
		return when (actionType)
		{
			ProtocolRequestTypeEnum.REQUEST_TYPE_NONE -> "none"
			ProtocolRequestTypeEnum.REQUEST_TYPE_NOTIFICATION -> "notification"
			ProtocolRequestTypeEnum.REQUEST_TYPE_SERVER -> "server"
		}
	}
}
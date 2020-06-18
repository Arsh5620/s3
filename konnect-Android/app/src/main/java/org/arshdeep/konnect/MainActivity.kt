package org.arshdeep.konnect

import android.app.AlertDialog
import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import android.widget.EditText
import android.widget.Toast
import com.google.android.material.snackbar.Snackbar
import androidx.appcompat.app.AppCompatActivity

import org.arshdeep.konnect.dbp.ProtocolCallback
import org.arshdeep.konnect.dbp.Protocol
import org.arshdeep.konnect.dbp.RequestPOJO
import org.arshdeep.konnect.dbp.ResponsePOJO

class MainActivity : AppCompatActivity()
{
	private val LOG_TAG = this::class.java.toString()
	private lateinit var rootView: View
	private lateinit var protocol: Protocol
	
	private var hostname: String = "localhost"
	private val port = 4096
	
	override fun onCreateOptionsMenu(menu: Menu): Boolean
	{
		val inflater: MenuInflater = menuInflater
		inflater.inflate(R.menu.main, menu)
		return true
	}
	
	override fun onCreate(savedInstanceState: Bundle?)
	{
		super.onCreate(savedInstanceState)
		setContentView(R.layout.activity_main)
		rootView = findViewById(R.id.root_view)
	}
	
	override fun onOptionsItemSelected(item: MenuItem): Boolean
	{
		return if (item.itemId == R.id.action_about_menu)
		{
			protocol.getServerInfo(object : ProtocolCallback
			{
				override fun callbackResponse(instance: ResponsePOJO)
				{
					var dialog: AlertDialog.Builder = AlertDialog.Builder(this@MainActivity);
					dialog.setMessage(instance.data);
					dialog.setTitle("About");
					dialog.create().show();
				}
			})
			true
		}
		else
		{
			super.onOptionsItemSelected(item)
		}
	}
	
	fun submitButtonClick(view: View)
	{
		var username = (findViewById<EditText>(R.id.username_edittext)).text.toString()
		var password = (findViewById<EditText>(R.id.password_edittext)).text.toString()
		var hostname = (findViewById<EditText>(R.id.hostname_edittext)).text.toString()
		
		var errorMessage: String? = null
		when
		{
			username.isNullOrBlank() ->
			{
				errorMessage = "Username cannot be empty."
			}
			password.isNullOrBlank() ->
			{
				errorMessage = "Password cannot be empty."
			}
			hostname.isNullOrBlank() ->
			{
				hostname = "localhost"
			}
		}
		
		if (!errorMessage.isNullOrBlank())
		{
			Toast.makeText(this, errorMessage, Toast.LENGTH_LONG).show()
			return
		}
		
		this.hostname = hostname
		this.protocol = Protocol(username, password, hostname, port, object : ProtocolCallback
		{
			override fun callbackResponse(instance: ResponsePOJO)
			{
				if (instance.request?.isInit!!)
				{
					Snackbar.make(
						rootView,
						"Successfully connected to the server",
						Snackbar.LENGTH_LONG
					)
						.setAction("", null)
						.show()
				}
				else
				{
					Snackbar.make(rootView, "Could not connect to server", Snackbar.LENGTH_INDEFINITE)
						.setAction("Retry?") {
							protocol.retryConnection(hostname, port)
						}
						.show()
				}
			}
		})
	}
}

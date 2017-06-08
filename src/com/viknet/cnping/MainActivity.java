package com.viknet.cnping;

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.widget.EditText;
import android.content.Context;
import android.widget.Button;
import android.view.View;
import android.util.Log;
import android.os.AsyncTask;

public class MainActivity extends Activity {
	
	static {
		System.loadLibrary("pinger");
	}

	private class SuSocketsRequest extends AsyncTask<Void, Void, Boolean> {
    	protected Boolean doInBackground(Void... params) {
    		return requestSockets();
    	}
    	protected void onPostExecute(Boolean result) {
    		if (result) return;

			new AlertDialog.Builder(MainActivity.this).setTitle("Error").setMessage("Cannot get sockets with su.").setPositiveButton("OK",
				new DialogInterface.OnClickListener() {
		            public void onClick(DialogInterface dialog, int id) {
		            	dialog.dismiss();
		            }}).create().show();
     	}
	}

	private EditText tv;
	private static native boolean requestSockets();
	private static native boolean startPing(String hostname);
	private static native void stopPing();

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	    setContentView(R.layout.activity_main);
	    // findViewById(R.id.imageView).setBackgroundColor(0xFF707070);
	    tv = (EditText) findViewById(R.id.textHostname);
	    new SuSocketsRequest().execute();

	    //TODO should be one button switching action
	    ((Button)findViewById(R.id.goButton)).setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				// tv.setText(startPing()?"Ping success":"Ping fail");
				startPing(tv.getText().toString());
			}
		});
		((Button)findViewById(R.id.stopButton)).setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				findViewById(R.id.imageView).postInvalidate();
				stopPing();
			}
		});
	}
}

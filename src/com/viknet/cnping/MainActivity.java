package com.viknet.cnping;

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.widget.EditText;
import android.widget.TextView;
import android.content.Context;
import android.widget.Button;
import android.view.View;
import android.view.KeyEvent;
import android.view.inputmethod.EditorInfo;
import android.util.Log;
import android.os.AsyncTask;
import android.view.inputmethod.InputMethodManager;

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
	private Button goButton;
	private static native boolean requestSockets();
	private static native boolean startPing(String hostname);
	private static native void stopPing();
	private boolean isRunning = false;

	protected void hideKeyboard(){
		InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		imm.hideSoftInputFromWindow(tv.getWindowToken(), 0);
		tv.clearFocus();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		getActionBar().setTitle("CNPing");
		setContentView(R.layout.activity_main);

		tv = (EditText) findViewById(R.id.textHostname);
		goButton = ((Button)findViewById(R.id.goButton));
		new SuSocketsRequest().execute();

		tv.setOnEditorActionListener(new TextView.OnEditorActionListener() {
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				if ((event != null && (event.getKeyCode() == KeyEvent.KEYCODE_ENTER)) || (actionId == EditorInfo.IME_ACTION_DONE))
					hideKeyboard();
				return false;
			}
		});

		goButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if(!isRunning){
					if (startPing(tv.getText().toString())){
						hideKeyboard();
						((Button)v).setText("STOP");
						isRunning = true;
					} else {
						new AlertDialog.Builder(MainActivity.this).setTitle("Error").setMessage("Ping error.").setPositiveButton("OK",
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog, int id) {
									dialog.dismiss();
								}}).create().show();
					}
				} else {
					stopPing();
					((Button)v).setText("GO");
					isRunning = false;
				}
			}
		});
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);

		getActionBar().show();
		if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE){
			getActionBar().hide();
			((View)findViewById(R.id.controlsArea)).setVisibility(View.GONE);
		} else {
			getActionBar().show();
			((View)findViewById(R.id.controlsArea)).setVisibility(View.VISIBLE);
		}
	}
}

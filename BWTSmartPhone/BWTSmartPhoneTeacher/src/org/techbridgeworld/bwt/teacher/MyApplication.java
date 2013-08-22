package org.techbridgeworld.bwt.teacher;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import org.apache.http.HttpResponse;
import org.apache.http.NameValuePair;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.message.BasicNameValuePair;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.app.Application;
import android.content.Context;
import android.hardware.SensorManager;
import android.os.AsyncTask;
import android.speech.tts.TextToSpeech;
import android.util.Log;

/**
 * MyApplication initializes and stores all global objects and variables.
 * MyApplication is created before any activity in the application.
 * 
 * @author neharathi
 */
public class MyApplication extends Application implements
		TextToSpeech.OnInitListener {

	// Speaks text aloud
	public TextToSpeech myTTS;

	// Detects the phone shaking
	public SensorManager myManager;
	public ShakeEventListener myListener;

	// The text given upon opening an activity
	public String prompt;

	// The text given upon shaking the phone
	public String help;

	// Represents the teachers' game selection
	public int game;

	// Represents the teachers' category selection
	public int category;

	// The teachers' sound selection
	public String sound;

	// The IP address of the server
	public String SERVER_ADDRESS = "http://128.237.196.208:3000";

	// Stores the Hangman words from the server
	public ArrayList<String> hangmanWords;

	@Override
	public void onCreate() {
		// Initializes myTTS, myManager, and myListener
		myTTS = new TextToSpeech(this, this);
		myManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
		myListener = new ShakeEventListener();

		// When the phone is shaken, speak the help text aloud
		myListener.setOnShakeListener(new ShakeEventListener.OnShakeListener() {
			public void onShake() {
				speakOut(help);
			}
		});

		// Populate hangmanWords
		new HTTPAsyncTask().execute();
	}

	@Override
	public void onInit(int status) {
		if (status == TextToSpeech.SUCCESS) {
			// Set up myTTS and speak the prompt text aloud
			myTTS.setLanguage(Locale.US);
			speakOut(prompt);
		} else {
			Log.e("TTS", "Initilization Failed!");
		}
	}

	/**
	 * Speaks text aloud using Text To Speech.
	 * 
	 * @param text
	 */
	public void speakOut(String text) {
		myTTS.speak(text, TextToSpeech.QUEUE_ADD, null);
	}

	/**
	 * HTTPAsyncTask populates hangmanWords on a background thread. To do this,
	 * it makes a POST request to the server with the admin's credentials. The
	 * server sends back a response containing the Hangman words.
	 * 
	 * @author neharathi
	 */
	public class HTTPAsyncTask extends AsyncTask<Void, Integer, Void> {
		@Override
		protected Void doInBackground(Void... params) {
			// Initialize HTTP Post
			HttpPost post = new HttpPost(SERVER_ADDRESS + "/login");

			// Initialize the POST parameters
			List<NameValuePair> nameValuePair = new ArrayList<NameValuePair>(2);
			nameValuePair.add(new BasicNameValuePair("username", "admin"));
			nameValuePair.add(new BasicNameValuePair("password", "admin"));

			// URL encode the POST parameters
			try {
				post.setEntity(new UrlEncodedFormEntity(nameValuePair));
			} catch (UnsupportedEncodingException e) {
				e.printStackTrace();
			}

			HttpResponse response = null;
			hangmanWords = new ArrayList<String>();
			try {
				HttpClient client = new DefaultHttpClient();
				HttpGet get = new HttpGet();
				get.setURI(new URI(SERVER_ADDRESS + "/words"));
				response = client.execute(get);
				populateHangmanWords(response.getEntity().getContent());
			} catch (URISyntaxException e) {
				e.printStackTrace();
			} catch (ClientProtocolException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
			return null;
		}
	}

	/**
	 * Populates hangmanWords using the response stream.
	 * 
	 * @param responseStream
	 */
	private void populateHangmanWords(InputStream responseStream) {
		// Convert responseStream to a JSON-encoded string (json)
		String json = "";
		if (responseStream != null) {
			Writer writer = new StringWriter();
			char[] buffer = new char[1024];
			try {
				Reader reader = new BufferedReader(new InputStreamReader(
						responseStream, "UTF-8"), 1024);
				int n;
				while ((n = reader.read(buffer)) != -1) {
					writer.write(buffer, 0, n);
				}
			} catch (IOException e) {
				e.printStackTrace();
			} finally {
				try {
					responseStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			json = writer.toString();
		}

		// Use json to create a JSONArray (hangmanJSON)
		JSONArray hangmanJSON = null;
		try {
			hangmanJSON = new JSONArray(json);
		} catch (JSONException e) {
			e.printStackTrace();
		}

		// Use hangmanJSON to populate hangmanWords
		if (hangmanJSON != null) {
			for (int i = 0; i < hangmanJSON.length(); i++) {
				JSONObject row;
				try {
					row = hangmanJSON.getJSONObject(i);
					hangmanWords.add(row.getString("word"));
				} catch (JSONException e) {
					e.printStackTrace();
				}

			}
		}
	}
}
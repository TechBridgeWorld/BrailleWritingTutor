package org.techbridgeworld.bwt.student;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
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
import java.util.HashMap;
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
import android.content.pm.PackageManager.NameNotFoundException;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.os.AsyncTask;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;
import android.speech.tts.UtteranceProgressListener;
import android.util.Log;

/**
 * MyApplication initializes and stores all global objects and variables.
 * MyApplication is created before any activity in the application.
 * 
 * @author neharathi
 */
public class MyApplication extends Application implements OnInitListener {

	// Speaks text aloud
	public TextToSpeech myTTS;
	HashMap<String, String> params;

	// The teacher application context
	public Context context;

	// Controls playing of audio files
	public MediaPlayer myPlayer;

	// The directory containing the audio files
	private String dir;

	// The list of audio files to be played
	public ArrayList<String> filenames;

	// The current audio file being played
	public int currentFile;

	// The text given upon opening an activity
	public String prompt;

	// The IP address of the server
	public final String SERVER_ADDRESS = "http://128.237.196.208:3000";

	// Stores the Hangman words from the server
	public ArrayList<String> hangmanWords;

	@Override
	public void onCreate() {
		// Initializes myTTS, myPlayer, and their associated variables
		myTTS = new TextToSpeech(this, this);
		params = new HashMap<String, String>();
		params.put(TextToSpeech.Engine.KEY_PARAM_UTTERANCE_ID, "utteranceId");

		myPlayer = new MediaPlayer();
		filenames = new ArrayList<String>();
		currentFile = 0;

		/*
		 * If the teacher app has been installed, set context and dir
		 * accordingly. Otherwise, set context to null.
		 */
		try {
			context = createPackageContext("org.techbridgeworld.bwt.teacher",
					MODE_PRIVATE);
			dir = context.getFilesDir().getPath().toString();
		} catch (NameNotFoundException e) {
			context = null;
			e.printStackTrace();
		}

		new HTTPAsyncTask().execute();
	}

	@Override
	public void onInit(int status) {
		if (status == TextToSpeech.SUCCESS) {
			myTTS.setOnUtteranceProgressListener(new UtteranceProgressListener() {
				@Override
				public void onDone(String utteranceId) {
					/*
					 * When myTTS is done speaking the current "audio file",
					 * call playAudio on the next audio file.
					 */
					if (utteranceId.equals("utteranceId")) {
						myTTS.stop();
						if (currentFile < filenames.size() - 1) {
							currentFile++;
							playAudio(filenames.get(currentFile));
						} else {
							filenames.clear();
							currentFile = 0;
						}
					}
				}

				@Override
				public void onError(String utteranceId) {
				}

				@Override
				public void onStart(String utteranceId) {
				}
			});

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
	 *            the text
	 */
	public void speakOut(String text) {
		myTTS.speak(text, TextToSpeech.QUEUE_ADD, params);
	}

	/**
	 * Adds a resourceId to filenames
	 * 
	 * @param resourceId
	 *            i.e. R.string.___
	 */
	public void queueAudio(int resourceId) {
		filenames.add(getResources().getString(resourceId));
	}

	/**
	 * Adds a string to filenames
	 * 
	 * @param str
	 *            the string
	 */
	public void queueAudio(String str) {
		filenames.add(str);
	}

	/**
	 * Clears everything in the audio queue
	 */
	public void clearAudio() {
		myTTS.stop();
		if (myPlayer.isPlaying()) {
			myPlayer.stop();
		}
		filenames.clear();
		currentFile = 0;
	}

	/**
	 * Starts playing the audio files in filenames
	 */
	public void playAudio() {
		if (filenames.isEmpty())
			return;
		playAudio(filenames.get(0));
	}

	/**
	 * Plays a certain audio file
	 * 
	 * @param filename
	 *            the audio file
	 */
	public void playAudio(String filename) {
		FileInputStream fis;
		try {
			filename = filename.replaceAll(" ", "_");
			fis = new FileInputStream(dir + "/" + filename + ".m4a");

			// If the audio file exists, use myPlayer to play it
			myPlayer.reset();
			myPlayer.setDataSource(fis.getFD());
			fis.close();
			myPlayer.prepare();
			myPlayer.start();
			myPlayer.setOnCompletionListener(new OnCompletionListener() {
				@Override
				public void onCompletion(MediaPlayer mp) {
					/*
					 * When myPlayer is done playing the current audio file,
					 * call playAudio on the next audio file.
					 */
					myPlayer.stop();
					if (currentFile < filenames.size() - 1) {
						currentFile++;
						playAudio(filenames.get(currentFile));
					} else {
						filenames.clear();
						currentFile = 0;
					}
				}
			});
		} catch (FileNotFoundException e) {
			// If the audio file does not exist, use text to speech to speak it
			filename = filename.replaceAll("_", " ");
			speakOut(filename);
		} catch (IllegalArgumentException e) {
			e.printStackTrace();
		} catch (IllegalStateException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
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
	 *            the response stream
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
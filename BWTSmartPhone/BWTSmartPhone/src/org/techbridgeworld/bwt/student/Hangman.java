package org.techbridgeworld.bwt.student;

import java.util.ArrayList;
import java.util.Date;
import java.util.Random;
import javaEventing.interfaces.Event;
import javaEventing.interfaces.GenericEventListener;

import org.techbridgeworld.bwt.api.BWT;
import org.techbridgeworld.bwt.api.events.SubmitEvent;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.util.Log;
import android.view.KeyEvent;

/**
 * Hangman replicates the functionality of Hangman on the BWT. This game takes
 * words from a wordbank off of the server and asks students to guess the word.
 * It keeps students updated about the current word using "dash" as blanks, and
 * reveals the word if the student reaches the maximum number of mistakes.
 * 
 * @author jessicalo
 */
public class Hangman extends Activity {

	// The global application
	private MyApplication application;

	// Speaks text aloud
	private TextToSpeech tts;

	// The BWT
	private final BWT bwt = new BWT(this, Hangman.this);

	// Randomly generates a new animal to be guessed
	private Random generator = new Random(new Date().getTime());

	// Mapping of 1-8 to their string counterparts
	private String[] numbers = { "one", "two", "three", "four", "five", "six",
			"seven", "eight" };

	// List of words taken from the server
	private String[] wordBank = {};

	// List of letters that have been guessed
	private ArrayList<Character> guessedLetters;

	// Maximum number of mistakes before the word is revealed
	private final int MAX_MISTAKES = 8;

	// Number of mistakes in a row
	private int mistakes;

	// The current word being guessed
	private String currWord = "";

	// The index of the current word in the word bank
	private int wordBankInd;

	// Represents the word as it is guessed
	private char[] wordStatus;

	// Number of correct letters that have been guessed
	private int numCorrectLetters;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.list);

		// Get the global application and global text to speech
		application = ((MyApplication) getApplicationContext());
		tts = application.myTTS;

		// Initialize currentFile and filenames
		application.currentFile = 0;
		application.filenames.clear();

		/*
		 * If hangmanWords is empty, alert the user and tell to ensure that the
		 * server is running and an Internet connection is established before
		 * going to GameActivity.
		 */
		if (application.hangmanWords.size() == 0) {
			AlertDialog.Builder builder = new AlertDialog.Builder(Hangman.this);
			builder.setMessage(R.string.no_words).setPositiveButton(
					R.string.ok, new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int id) {
							Intent intent = new Intent(Hangman.this,
									GameActivity.class);
							startActivity(intent);
						}
					});
			AlertDialog dialog = builder.create();
			dialog.show();
		}

		// Populate the word bank with the words from the server
		wordBank = (String[]) application.hangmanWords
				.toArray(new String[application.hangmanWords.size()]);

		// Initialize the BWT connection.
		bwt.init();

		// Start the BWT
		bwt.start();

		// Start tracking the state of the BWT
		bwt.initializeEventListeners();
		bwt.startTracking();
		runGame();
	}

	@Override
	public void onPause() {
		// Clear the audio queue and stop the BWT
		application.clearAudio();
		bwt.stopTracking();
		bwt.removeEventListeners();
		bwt.stop();
		super.onPause();
	}

	@Override
	public void onDestroy() {
		// Stop and shutdown text to speech
		if (tts != null) {
			tts.stop();
			tts.shutdown();
		}
		super.onDestroy();
	}

	/**
	 * Provides the first instructions and creates listeners for the BWT board.
	 */
	private void runGame() {
		// Generate the first word
		wordBankInd = -1;
		regenerate();

		// Provide first instructions
		application.playAudio();

		// Create a listener for the BWT board
		createListeners();
	}

	/**
	 * Randomly generates a new word from the word bank to be guessed and queues
	 * the associated audio.
	 */
	private void regenerate() {
		// Update wordBankInd
		wordBankInd++;

		// Reset mistakes and guessedLetters
		mistakes = 0;
		guessedLetters = new ArrayList<Character>();

		// If the game is over, restart it
		if (wordBankInd >= wordBank.length) {
			wordBankInd = -1;
			return;
		}

		/*
		 * Randomly generate the next word such that it hasn't been guessed
		 * before.
		 */
		int nextWordInd = generator.nextInt(wordBank.length - wordBankInd)
				+ wordBankInd;
		currWord = wordBank[nextWordInd];
		numCorrectLetters = 0;

		/*
		 * Tell the user:
		 * "The new word has [number] letters: dash, dash, .... Guess a letter."
		 */
		int numLetters = currWord.length();
		application.queueAudio(R.string.the_new_word);
		application.queueAudio(numbers[numLetters - 1]);
		application.queueAudio(R.string.letters);
		wordStatus = new char[numLetters];
		for (int i = 0; i < numLetters; i++) {
			wordStatus[i] = '-';
			application.queueAudio(R.string.dash);
		}
		application.queueAudio(R.string.guess_a_letter);

		/*
		 * Swap the words in wordBank such that everything before wordBankInd
		 * has been guessed.
		 */
		wordBank[nextWordInd] = wordBank[wordBankInd];
		wordBank[wordBankInd] = currWord;
	}

	/**
	 * Updates the user on the status of the word and the number of mistakes
	 * they've made before prompting them to guess a new letter. Called when
	 * game is ready for the user to guess a letter.
	 */
	private void promptGuess() {
		/*
		 * Tell the user: "So far the word is: [dash] [letter] [dash] ..." where
		 * a dash indicates that they have yet to guess that letter.
		 */
		application.queueAudio(R.string.so_far);
		for (int i = 0; i < currWord.length(); i++) {
			Character c = wordStatus[i];
			if (c == '-') {
				application.queueAudio(R.string.dash);
			} else {
				application.queueAudio(c.toString());
			}
		}

		/*
		 * Tell the user: "You've made [number] mistake(s)."
		 */
		application.queueAudio(R.string.youve_made);
		if (mistakes == 1) {
			application.queueAudio(R.string.one);
			application.queueAudio(R.string.mistake);
		} else {
			application.queueAudio(((Integer) mistakes).toString());
			application.queueAudio(R.string.mistakes);
		}

		/*
		 * If the user has one mistake left, tell the user:
		 * "But you have one last chance to guess the entire word."
		 */
		if (mistakes == MAX_MISTAKES - 1) {
			application.queueAudio(R.string.but_you_have);
		}

		/*
		 * Tell the user: "Guess a letter."
		 */
		application.queueAudio(R.string.guess_a_letter);
	}

	/**
	 * Handles the guessed letter being in the current word.
	 */
	private void correctGuessHandler(char guessedLetter) {
		// Update guessedLetters and wordStatus
		guessedLetters.add(guessedLetter);
		int start = 0;
		while (start >= 0 && start < currWord.length()) {
			int index = currWord.indexOf(guessedLetter, start);
			if (index < 0)
				break;

			numCorrectLetters++;
			wordStatus[index] = guessedLetter;
			start = index + 1;
		}

		/*
		 * If the word is complete, tell the user and reveal the word before
		 * randomly generating the next word. Otherwise, update and re-prompt
		 * the user.
		 */
		if (numCorrectLetters == currWord.length()) {
			application.queueAudio(R.string.good);

			revealCurrWord();
			regenerate();
		} else {
			promptGuess();
		}
	}

	/**
	 * Reveals the current word to the user.
	 */
	private void revealCurrWord() {
		/*
		 * Tell the user:
		 * "The correct answer was [word]. [letter] [letter] ...."
		 */
		application.queueAudio(R.string.the_correct_answer_was);
		application.queueAudio(currWord);
		for (int i = 0; i < currWord.length(); i++) {
			Character ch = currWord.charAt(i);
			application.queueAudio(ch.toString());
		}

	}

	/**
	 * Handles the guessed letter not being in the current word based on the
	 * number of mistakes.
	 */
	private void wrongGuessHandler() {
		// Update mistakes and tell the user they were wrong
		mistakes++;
		application.queueAudio(R.string.no);

		/*
		 * If the user has had three mistakes, reveal the current word and
		 * randomly generate a new word. Otherwise, update and re-prompt the
		 * user.
		 */
		if (mistakes == MAX_MISTAKES) {
			revealCurrWord();
			regenerate();
		} else {
			promptGuess();
		}
	}

	/**
	 * Processes every glyph written and queues/plays audio as necessary.
	 */
	private void createListeners() {
		// Compares the expected word to the user input
		GenericEventListener HangmanListener = new GenericEventListener() {
			@Override
			public void eventTriggered(Object arg0, Event arg1) {
				bwt.defaultSubmitHandler(arg0, arg1);
				SubmitEvent e = (SubmitEvent) arg1;
				Log.d("Hangman", "Triggered Submit Event");

				// Get the finished glyph and reset the bits at that cell to 0
				int cellInd = e.getCellInd();
				char glyphAtCell = bwt.getGlyphAtCell(cellInd);
				bwt.setBitsAtCell(cellInd, 0);

				/*
				 * If the glyph is invalid, tell the user so and handle it as a
				 * wrong guess.
				 */
				if (glyphAtCell == '-') {
					application.queueAudio(R.string.invalid_input);
					wrongGuessHandler();
					application.playAudio();
					return;
				}

				// Speak the character aloud
				application.queueAudio(((Character) glyphAtCell).toString());

				/*
				 * If the letter has already been guessed, tell the user:
				 * "You've already guessed that letter." Then, update and
				 * re-prompt the user. Otherwise, update guessedLetters.
				 */
				if (guessedLetters.contains(glyphAtCell)) {
					application.queueAudio(R.string.youve_already);
					promptGuess();
					application.playAudio();
					return;
				} else {
					guessedLetters.add(glyphAtCell);
				}

				/*
				 * Compare the glyph to the expected character and handle the
				 * result accordingly.
				 */
				if (currWord.indexOf(glyphAtCell) < 0)
					wrongGuessHandler();
				else
					correctGuessHandler(glyphAtCell);

				application.playAudio();
			}
		};

		bwt.replaceListener("onSubmitEvent", HangmanListener);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// If the user presses back, go to GameActivity
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			bwt.removeEventListeners();
			Intent intent = new Intent(Hangman.this, GameActivity.class);
			startActivity(intent);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
}
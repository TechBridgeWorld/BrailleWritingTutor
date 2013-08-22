package org.techbridgeworld.bwt.student;

import java.util.Arrays;
import java.util.Collections;
import javaEventing.interfaces.Event;
import javaEventing.interfaces.GenericEventListener;

import org.techbridgeworld.bwt.api.BWT;
import org.techbridgeworld.bwt.api.events.BoardEvent;
import org.techbridgeworld.bwt.api.libs.Braille;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.view.KeyEvent;

/**
 * LearnLetters replicates the functionality of Learn Letters on the BWT. This
 * game teaches students how to write Braille letters in groups of five. That
 * is, it teaches students five letters and then tests students on these five
 * letters in a random order before moving on to the next group.
 * 
 * @author jessicalo
 * 
 */
public class LearnLetters extends Activity {
	// The global application
	private MyApplication application;

	// Speaks text aloud
	private TextToSpeech tts;

	// The BWT
	private final BWT bwt = new BWT(this, LearnLetters.this);

	// Contains mappings of dots in a Braille cell to a character
	private static final Braille braille = new Braille();

	// Mapping of 1-6 to their string counterparts
	private String[] numbers = { "one", "two", "three", "four", "five", "six" };

	// Grouping of letters to be taught/tested
	private static final char[][] letters = { { 'a', 'b', 'c', 'd', 'e' },
			{ 'f', 'g', 'h', 'i', 'j' }, { 'k', 'l', 'm', 'n', 'o' },
			{ 'p', 'q', 'r', 's', 't' }, { 'u', 'v', 'w', 'x', 'y', 'z' } };

	// Indices of letters in a shuffled order for testing mode
	private static int[][] shuffledIndices;

	// Maximum number of mistakes before re-teaching a letter in testing mode
	private static final int MAX_MISTAKES = 3;

	// Group index in letters being taught/tested
	private int groupInd;

	// Letter index in letters[groupInd] being taught/tested
	private int countLetterInd;

	// Expected Braille input in binary form
	private int expectedBrailleCode;

	// Number of mistakes in a row (for testing mode)
	private int mistakes;

	// Tracks whether game is in teaching or testing mode
	private boolean teaching;

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
	 * Provides the first instructions and creates a listener for the BWT board.
	 */
	private void runGame() {
		// Create shuffledIndices for testing mode
		shuffleIndices();

		// Initialize the game variables
		groupInd = 0;
		mistakes = 0;
		countLetterInd = 0;
		teaching = true;

		// Provide first instructions
		spellLetterInstruction(groupInd, countLetterInd);
		application.playAudio();

		// Create a listener for the BWT board
		BWT.getBoard().setBitsAtUnivCell(0);
		expectedBrailleCode = braille.get(letters[groupInd][countLetterInd]);
		createListener();
	}

	/**
	 * Checks the onBoardEvent trigger to see if the letter is correct, wrong,
	 * or in progress and queues/plays audio as necessary.
	 */
	private void createListener() {
		bwt.replaceListener("onBoardEvent", new GenericEventListener() {
			@Override
			public void eventTriggered(Object sender, Event event) {

				// Ignore AltBtn being pressed
				if (((BoardEvent) event).getCellInd() == -1)
					return;

				int currentBrailleCode = BWT.getBoard().getBitsAtUnivCell();

				// If extra bits are set in c and not e, ((c & e) ^ c) > 0
				boolean isWrong = (((currentBrailleCode & expectedBrailleCode) ^ currentBrailleCode) > 0);

				// If the user is correct, tell them so and test the next letter
				if (currentBrailleCode == expectedBrailleCode) {
					application.queueAudio(R.string.good);
					testNextLetter();
				}
				/*
				 * Otherwise, if the user is wrong, tell them so and re-test the
				 * current letter.
				 */
				else if (isWrong) {
					application.queueAudio(R.string.no);
					testCurrLetter();
				}
				// Otherwise, f the user is in progress, do nothing
				else {
					return;
				}

				application.playAudio();
			}
		});
	}

	/**
	 * Tests the next letter. Called when the user is correct.
	 */
	private void testNextLetter() {
		BWT.getBoard().setBitsAtUnivCell(0);

		// Update wrongAttemps and countLetterInd
		mistakes = 0;
		countLetterInd++;

		/*
		 * If the user is at the end of the current group of letters, check if
		 * they are in testing mode. If so, go to the next group. Regardless of
		 * mode, reset countLetterInd and switch modes.
		 */
		if (countLetterInd >= letters[groupInd].length) {
			// If the user is in testing mode, go to the next group
			if (!teaching) {
				groupInd++;
				if (groupInd >= letters.length) {
					// If the game is over, restart it.
					groupInd = 0;
					teaching = true;
					countLetterInd = 0;
					mistakes = 0;
					expectedBrailleCode = braille
							.get(letters[groupInd][countLetterInd]);
					spellLetterInstruction(groupInd, countLetterInd);
					return;
				}
			}
			// Regardless of mode, reset countLetterInd and switch modes
			countLetterInd = 0;
			teaching = !teaching;
		}

		// Set the letter index depending on the mode
		int letterInd = teaching ? countLetterInd
				: shuffledIndices[groupInd][countLetterInd];

		/*
		 * If the user is in testing mode, provide testing instructions.
		 * Otherwise, provide spelling instructions.
		 */
		if (!teaching)
			testLetterInstruction(groupInd, letterInd);
		else
			spellLetterInstruction(groupInd, letterInd);

		expectedBrailleCode = braille.get(letters[groupInd][letterInd]);
	}

	/**
	 * Re-tests the current letter and updates mistakes. Called when the user is
	 * incorrect.
	 */
	private void testCurrLetter() {
		BWT.getBoard().setBitsAtUnivCell(0);

		// Update mistakes
		mistakes++;

		// Set the letter index depending on the mode
		int letterInd = teaching ? countLetterInd
				: shuffledIndices[groupInd][countLetterInd];

		/*
		 * If the user is in teaching mode or has had three mistakes, provide
		 * spelling instructions. Otherwise, if the user is in testing mode,
		 * provide testing instructions.
		 */
		if (teaching || mistakes == MAX_MISTAKES)
			spellLetterInstruction(groupInd, letterInd);
		else if (!teaching)
			testLetterInstruction(groupInd, letterInd);
	}

	/**
	 * Provides instruction
	 * "To write the letter [letter] please press [number], [number], ..."
	 * 
	 * @param groupInd
	 *            the index of the group
	 * @param letterInd
	 *            the index of the letter
	 */
	private void spellLetterInstruction(int groupInd, int letterInd) {
		char let = letters[groupInd][letterInd];
		int btns = braille.get(let);
		StringBuffer btnStrBuf = new StringBuffer("");
		for (int i = 0; i < 6; i++) {
			if ((btns & (1 << i)) > 0) {
				btnStrBuf.append(i + 1 + " ");
			}
		}

		application.queueAudio(R.string.to_write_the_letter);
		application.queueAudio(((Character) let).toString());
		application.queueAudio(R.string.please_press);
		String[] buttons = btnStrBuf.toString().split(" ");
		for (int i = 0; i < buttons.length; i++)
			application.queueAudio(numbers[Integer.parseInt(buttons[i]) - 1]);
	}

	/**
	 * Provides instruction "Please write [letter]."
	 * 
	 * @param groupInd
	 *            the index of the group
	 * @param letterInd
	 *            the index of the letter
	 */
	private void testLetterInstruction(int groupInd, int letterInd) {
		char letter = letters[groupInd][letterInd];

		application.queueAudio(R.string.please_write);
		application.queueAudio(((Character) letter).toString());
	}

	/**
	 * Create shuffledIndices for testing mode.
	 */
	private void shuffleIndices() {
		// Initialize shuffledIndices
		shuffledIndices = new int[letters.length][];
		for (int i = 0; i < letters.length; i++) {
			shuffledIndices[i] = new int[letters[i].length];
			for (int j = 0; j < letters[i].length; j++) {
				shuffledIndices[i][j] = j;
			}
		}

		// Shuffle the indices in shuffledIndices
		for (int i = 0; i < letters.length; i++) {
			Collections.shuffle(Arrays.asList(shuffledIndices[i]));
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// If the user presses back, go to GameActivity
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			bwt.removeEventListeners();
			Intent intent = new Intent(LearnLetters.this, GameActivity.class);
			startActivity(intent);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
}
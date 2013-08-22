package org.techbridgeworld.bwt.api;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import javaEventing.EventManager;
import javaEventing.interfaces.Event;
import javaEventing.interfaces.GenericEventListener;

import org.techbridgeworld.bwt.api.events.AltBtnEvent;
import org.techbridgeworld.bwt.api.events.BoardEvent;
import org.techbridgeworld.bwt.api.events.CellsEvent;
import org.techbridgeworld.bwt.api.events.MainBtnEvent;
import org.techbridgeworld.bwt.api.events.SubmitEvent;
import org.techbridgeworld.bwt.api.libs.Braille;

import android.app.Activity;
import android.content.Context;
import android.hardware.usb.UsbManager;
import android.os.Handler;
import android.util.Log;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

/**
 * BWT.java is in charge of opening and closing the communication with the BWT.
 * This class also updates Board.java, handles the triggering of events, and
 * acts as the middleman between the student applications and board.java
 * 
 * @author Jessica and Salem
 *
 */
public class BWT {

	// App Context
	private Context context;

	// BWT information/state
	private static Board board;
	private static final Braille braille = new Braille();
	private boolean isTracking;

	// Constants
	private static final int BAUDRATE = 57600;
	private static final int TIMEOUT = 1000;
	private static final int DEBOUNCE = 300; 	// Milliseconds to disable a button
												// for.
	private static final int INACTIVE_TIME = 5000;	//time (ms) of inactivity
													//before triggering SubmitEvent
	
	// Used for differentiating submit method for SubmitEvent
	public final int SUBMIT_TYPE_ALT	= 0;
	public final int SUBMIT_TYPE_CHANGE	= 1;
	public final int SUBMIT_TYPE_TIME	= 2;

	// Buffer / Debounce stuff
	private char[] dataBuffer = new char[6];
	private int bufferIdx = 0;
	private Hashtable<String, Boolean> debounceHash = new Hashtable<String, Boolean>();

	// USB connections
	private UsbManager usbManager;
	private UsbSerialDriver usbDriver;
	private SerialInputOutputManager serialManager;

	// Debounce handler for filtering bytes from the BWT
	private Handler debounceHandler = new Handler();
	
	// Handler and Runnable to keep track of inactivity
	private Handler inactivityHandler = new Handler();
	private Runnable inactivityRunnable;
	
	private final ExecutorService executor = Executors
			.newSingleThreadExecutor();
	private final SerialInputOutputManager.Listener listener = new SerialInputOutputManager.Listener() {

		@Override
		public void onRunError(Exception e) {
			// Ignore
			Log.e("DataTransfer",
					"In the onRunError function: " + e.getMessage());
		}

		@Override
		public void onNewData(final byte[] data) {
			updateReceivedData(data);
		}
	};

	/**
	 * BWT class contains all the interaction done between the main activities
	 * and the board/cell classes.
	 * 
	 * @param context
	 * @param activity
	 */
	public BWT(Context context, Activity activity) {
		this.context = context;
		BWT.board = new Board();
		this.isTracking = false;
		inactivityRunnable = null;
	}

	/**
	 * Init function should be called onCreate of the BWT.
	 */
	public void init() {
		usbManager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
		startIoManager();
	}

	/**
	 * BWT.start() should be called after BWT.init(). Will open the serial
	 * connection and create default event listeners
	 */
	public void start() {
		usbDriver = UsbSerialProber.acquire(usbManager);
		if (usbDriver != null) {
			try {
				usbDriver.open();
				usbDriver.setBaudRate(BAUDRATE);
				byte[] bt = "bt".getBytes();
				usbDriver.write(bt, TIMEOUT);
			} catch (IOException e) {
				try {
					Log.e("Connecting",
							"Error starting USB driver, attempting to close.");
					usbDriver.close();
				} catch (IOException e2) {
					Log.e("Connecting", "Unable to close after failed attempt to start.");
				}
				usbDriver = null;
				return;
			}
		}

		stateChange();
	}

	/**
	 * BWT.stop() should be called each time you leave the app or no longer need
	 * to read input from BWT. Also removes all event listeners, including
	 * developer's
	 */
	public void stop() {
		stopIoManager();
		if (usbDriver != null) {
			try {
				removeEventListeners();
				usbDriver.setBaudRate(BAUDRATE);
				byte[] bt = "bt".getBytes();
				usbDriver.write(bt, TIMEOUT);
				usbDriver.close();
			} catch (IOException e) {
				Log.e("Connecting", "bwt.stop() IO exception: " + e);
			}
			usbDriver = null;
		}
	}

	/**
	 * Start IO
	 */
	private void startIoManager() {
		if (usbDriver != null) {
			serialManager = new SerialInputOutputManager(usbDriver, listener);
			executor.submit(serialManager);
		} else {
			Log.e("Connecting", "usbDriver == null");
		}
	}

	/**
	 * Stop IO
	 */
	private void stopIoManager() {
		if (serialManager != null) {
			serialManager.stop();
			serialManager = null;
		}
	}

	/**
	 * Restarts IO
	 */
	public void stateChange() {
		stopIoManager();
		startIoManager();
	}

	
	/**
	 * Manages the debounce hashtable for seeing whether it is a new press or
	 * different
	 * 
	 * @param key
	 *            is a byte array
	 */
	private void debounceKey(String key) {
		final String newKey = key;
		debounceHash.put(newKey, true);

		// Start a runnable to un-block the key after a set time.
		Runnable r = new Runnable() {
			@Override
			public void run() {
				try {
					Thread.sleep(DEBOUNCE);
				} catch (InterruptedException e) {
					Log.e("Debounce", "Failed to sleep thread.");
					e.printStackTrace();
				}
				debounceHash.put(newKey, false);
			}
		};
		debounceHandler.post(r);
	}

	/**
	 * Returns true if a key is currently being ignored.
	 * 
	 * @param key to check
	 * @return
	 */
	private boolean isDebounced(String key) {
		boolean query = (debounceHash.get(key) == null ?
				false : debounceHash.get(key));
		return query;
	}

	/**
	 * Takes the received data, checks to see if it should be ignored/debounced,
	 * and triggers the corresponding events
	 * 
	 * @param data
	 */
	private void updateReceivedData(byte[] data) {
		// For every byte in the incoming data...
		for (int i = 0; i < data.length; i++) {
			// If we are done, and if the buffer represents a non-blocked key,
			// then log the buffer, clear it, and set its index to 0.
			// Note: the BWT passes 'n' to denote the end of the signal
			if (data[i] == 'n' || data[i] == 'N' || data[i] == 't') {
				// Catch the initial "bt" received from the device.
				if (data[i] == 't') {
					dataBuffer[bufferIdx] = (char)data[i];
					bufferIdx++;
				}

				StringBuilder message = new StringBuilder();
				message.append(dataBuffer, 0, bufferIdx);

				// Fire a trigger!
				if (!isDebounced(message.toString())) {
					Log.i("DataTransfer", "Fired trigger '" + message + "'");

					triggerNewDataEvent(message.toString());
					debounceKey(message.toString());

				} else {
					// The button has been debounced, don't fire. 
					Log.d("DataTransfer", "Button press blocked!");
				}

				bufferIdx = 0;
				// Zero out data buffer.
				for(int j = 0; j < 6; j++) {
					dataBuffer[j] = 0;
				}

			} else {
				// If the buffer is full and we haven't seen an 'n' yet, something is wrong.
				if (bufferIdx >= 6) {
					Log.e("DataTransfer", "bufferIdx out of range: " + bufferIdx);
				} else {
					// Add the passed character to the buffer and increment the buffers idx.
					dataBuffer[bufferIdx] = (char)data[i];
					bufferIdx++;
				}
			}
		}
	}

	/**
	 * Trigger the SubmitEvent every time INACTIVE_TIME is up
	 */
	private void startInactivityTimer() {
		inactivityRunnable = new Runnable() {
			@Override
			public void run() {
				//Trigger the SubmitEvent (if last cell wasn't altBtn) since time's up
				Integer cell = board.getCurrCellInd();
				if(cell != -1) {
					SubmitEvent submitEvt = new SubmitEvent(cell,
							board.getBitsAtCell(cell), SUBMIT_TYPE_TIME);
					EventManager.triggerEvent(this, submitEvt, "onSubmitEvent");
				}
			}
		};
		inactivityHandler.postDelayed(inactivityRunnable, INACTIVE_TIME);
	}
	
	/**
	 * Reset the timer; called each time user input is received
	 */
	private void resetInactivityTimer() {
		if(inactivityRunnable == null) {
			Log.e("Inactivity Timer", "Calling resetInactivityTimer without start (instantiating Runnable)");
			return;
		}
		inactivityHandler.removeCallbacks(inactivityRunnable);
		inactivityHandler.postDelayed(inactivityRunnable, INACTIVE_TIME);
	}
	
	/**
	 * Removes the triggering of SubmitEvent on timer
	 */
	private void stopInactivityTimer() {
		inactivityHandler.removeCallbacks(inactivityRunnable);
		inactivityRunnable = null;
	}
	
	
	/** Board getter */
	public static Board getBoard() {
		return board;
	}

	/**
	 * Allows event handlers to go off; updates board's state
	 */
	public void startTracking() {
		isTracking = true;
		startInactivityTimer();
	}

	/**
	 * Disregards changing state of board if stopped tracking
	 * 
	 * @return inputBuffer's remaining content
	 */
	public String stopTracking() {
		isTracking = false;
		stopInactivityTimer();
		return board.viewAndEmptyAsInputted();
	}

	/**
	 * Returns and empties everything in current 'buffer'
	 * 
	 * @return what was left in the buffer If not tracking, returns null
	 */
	public String dumpTrackingAsString() {
		if(!isTracking) return null;
		return board.viewAndEmptyAsInputted();
	}
	
	/**
	 * Returns everything in current 'buffer' (Does NOT empty)
	 * 
	 * @return what was left in the buffer If not tracking, returns null
	 */
	public String viewTrackingAsString() {
		if (!isTracking) return null;
		return board.viewAsInputted();
	}

	/**
	 * Dumps as an Array of Integers
	 * 
	 * @return
	 */
	public ArrayList<Integer> dumpTrackingAsBits() {
		if (!isTracking)
			return null;
		return board.viewAndEmptyBitsAtInputtedCells();
	}

	/**
	 * Views as an Array of Integers
	 * 
	 * @return
	 */
	public ArrayList<Integer> viewTrackingAsBits() {
		if (!isTracking)
			return null;
		return board.viewBitsAtInputtedCells();
	}

	/**
	 * Returns and empties everything in current 'buffer'
	 *  EXCEPT for current cell
	 * @return what was left in the buffer If not tracking, returns null
	 */
	public String dumpTrackingAsStringExceptCurrent() {
		if(!isTracking) return null;
		return board.viewAndEmptyAsInputtedExceptCurrent();
	}
	
	/**
	 * Returns everything in current 'buffer' (Does NOT empty)
	 *  EXCEPT for current cell
	 * @return what was left in the buffer If not tracking, returns null
	 */
	public String viewTrackingAsStringExceptCurrent() {
		if (!isTracking) return null;
		return board.viewAsInputtedExceptCurrent();
	}

	/**
	 * Dumps as an Array of Integers
	 * EXCEPT for current cell
	 * @return
	 */
	public ArrayList<Integer> dumpTrackingAsBitsExceptCurrent() {
		if (!isTracking)
			return null;
		return board.viewAndEmptyBitsAtInputtedCellsExceptCurrent();
	}

	/**
	 * Views as an Array of Integers
	 * EXCEPT for current cell
	 * @return
	 */
	public ArrayList<Integer> viewTrackingAsBitsExceptCurrent() {
		if (!isTracking)
			return null;
		return board.viewBitsAtInputtedCellsExceptCurrent();
	}
	
	/**
	 * Clears and resets state of the board
	 */
	public void resetBoard() {
		board.clearBoard();
	}

	/**
	 * Clears only touched inputs and resets state of the board
	 */
	public void clearTouchedCells() {
		board.clearTouchedCells();
	}
	
	/**
	 * @return the bits pressed for 'lastCell'
	 */
	public int getCurrentCellBits() {
		Integer lastCell = board.getCurrCellInd();
		if (lastCell < 0)
			return 0;
		return board.getBitsAtCell(lastCell);
	}
	
	/**
	 * Sets the bits for certain cell
	 * @return false if unsuccessful
	 */
	public boolean setBitsAtCell(int cellInd, int binVal) {
		if (cellInd < 0)
			return false;
		board.setBitsAtCell(cellInd, binVal);
		return true;
	}

	/**
	 * @return the current char for 'lastCell'
	 */
	public char getCurrentCellGlyph() {
		Integer lastCell = board.getLastInputInfoInd();
		return board.getGlyphAtCell(lastCell);
	}
	
	/**
	 * @return the char for given cell
	 */
	public char getGlyphAtCell(int cellInd) {
		return board.getGlyphAtCell(cellInd);
	}

	/**
	 * Compares current input with given char Takes into account bits of
	 * lastCell
	 * 
	 * @param c		: the glyph you're aiming to match
	 * @return true : if off track of matching c
	 */
	public boolean offTrackFromChar(char c) {
		Integer lastCell = board.getLastInputInfoInd();
		if (lastCell < 0) {
			Log.d("Check input", "Char off track; lastCell = " + lastCell);
			return false;
		}
		int currBits = board.getBitsAtCell(lastCell);
		int expBits = braille.get(c);
		return (((currBits & expBits) ^ currBits) > 0);
	}

	/**
	 * Checks if the board's input is on/off track of the given string s
	 * (takes into account everything in inputBuffer AND lastCell)
	 * 
	 * @param s
	 * @return true if the board's input is off track from s; false otherwise
	 */
	public boolean offTrackFromString(String s) {
		// Check what's in trackingBuffer
		String finishedDump = viewTrackingAsStringExceptCurrent();
		
		int indexFound = s.indexOf(finishedDump);
		if (indexFound != 0 && finishedDump.length() > 0) {
			Log.d("Check input", "Is off track; finishedDump = '" + finishedDump + "'");
			return true;
		}

		// Check the most recent cell
		return offTrackFromChar(s.charAt(finishedDump.length()));
	}

	/**
	 * Checks if the last cell's braille input matches the expected char c
	 * @param c
	 * @return true if matches
	 */
	public boolean currentMatchesChar(char c) {
		Integer lastCell = board.getLastInputInfoInd();
		if (lastCell < 0)
			return false;
		int currBits = board.getBitsAtCell(lastCell);
		int expBits = braille.get(c);
		return (currBits == expBits);

	}
	
	/**
	 * Used to find the last typed char that's done, the moment it's done,
	 * in a series of characters expected (String s)
	 * 
	 * @param s is the String you're comparing input with
	 * @return index of the last character that matches part of s
	 * Returns -1 if character not done, or if not even on right track
	 */
	public int currentMatchingIndexOfString(String s) {
		// Check what's in trackingBuffer
		String currDump = viewTrackingAsStringExceptCurrent();
		int indexFound = s.indexOf(currDump);
		
		if (currDump.equals(s))
			return currDump.length() - 1;
		if (indexFound != 0 && currDump.length() > 0)
			return -1;

		// Check the most recent cell
		int index = currDump.length();
		char ch = s.charAt(index);
		if(currentMatchesChar(ch))
			return index;
		else return -1;
	}
	
	/**
	 * Determines if input matches expected String
	 * @param s
	 * @return true of current input matches string s
	 */
	public boolean currentMatchesString(String s) {
		return (currentMatchingIndexOfString(s) == s.length() -1);
	}
	
	/**
	 * Called by updateReceivedData to trigger necessary events
	 * 
	 * @param String
	 *            sent from BWT firmware through USB
	 */
	private void triggerNewDataEvent(String message) {
		if (!isTracking)
			return;

		//Reset timer of inactivity
		resetInactivityTimer(); 
		
		// Remove terminating 'n's from commands.
		message = message.toLowerCase(Locale.getDefault()).replaceAll("n", "").trim();
		
		// Catch extraneous messages
		if (message.equals("bt") || message.length() == 0)
			return;
		
		Integer lastCell = board.getCurrCellInd();

		//First, update board
		board.handleNewInput(message);
		
		//Then, trigger events, based on what message was
		String referenceStr = "abcdefg";

		int currCell = -1;
		int currCellBits = 0; // The current set bits of currCell
		int currDot = -1; 		// The dot just hit

		// See if it's a, b-g, or two numbers
		if (referenceStr.indexOf(message) == 0) {
			EventManager.triggerEvent(this, new AltBtnEvent(message),
					"onAltBtnEvent");
			
		} else if (referenceStr.indexOf(message) > 0) {
			MainBtnEvent mainBtnEvent = new MainBtnEvent(message, board);
			EventManager.triggerEvent(this, mainBtnEvent, "onMainBtnEvent");
			currCell = 0;
			currDot = mainBtnEvent.getDot();
			
		} else {
			CellsEvent cellsEvent = new CellsEvent(message, board);
			EventManager.triggerEvent(this, cellsEvent, "onCellsEvent");

			currCell = cellsEvent.getCell();
			currDot = cellsEvent.getDot();
		}

		// Determine if there has been a cell change (including submit with altBtn)
		//(Event Handler updates lastCell)
		if (currCell != lastCell && lastCell != -1) {
			int submitType;
			//Hitting submit btn vs. changing cells
			if(currCell == -1)	submitType = SUBMIT_TYPE_ALT;
			else				submitType = SUBMIT_TYPE_CHANGE;
			EventManager.triggerEvent(this,
					new SubmitEvent(lastCell, board.getBitsAtCell(lastCell), submitType),
					"onSubmitEvent");
		}

		if (currCell >= 0)
			currCellBits = board.getBitsAtCell(currCell);

		// Trigger board event regardless
		EventManager.triggerEvent(this, new BoardEvent(message, currCell,
				currCellBits, currDot), "onBoardEvent");
	}

	/**
	 * Registers default event handlers; called in bwt.start();
	 */
	public void initializeEventListeners() {
		EventManager.registerEventListener("onAltBtnEvent",
				createOnAltBtnListener(), AltBtnEvent.class);
		
		EventManager.registerEventListener("onSubmitEvent",
				createOnSubmitListener(), SubmitEvent.class);
	}

	/**
	 * Unregisters all event listeners; called in bwt.stop();
	 */
	public void removeEventListeners() {
		EventManager.unregisterAllEventListenersForContext("onBoardEvent");
		EventManager.unregisterAllEventListenersForContext("onMainBtnEvent");
		EventManager.unregisterAllEventListenersForContext("onAltBtnEvent");
		EventManager.unregisterAllEventListenersForContext("onCellsEvent");
		EventManager.unregisterAllEventListenersForContext("onSubmitEvent");
	}

	/**
	 * Lets developers add their own event listeners; replaces current listeners
	 * Returns false if didn't recognize context; returns true otherwise
	 * 
	 * @param context
	 * @param customizedListener
	 * @return
	 */
	public boolean replaceListener(String context,
			GenericEventListener customizedListener) {
		Class<? extends Event> c = null;
		if (context == "onBoardEvent")
			c = BoardEvent.class;
		else if (context == "onAltBtnEvent")
			c = AltBtnEvent.class;
		else if (context == "onMainBtnEvent")
			c = MainBtnEvent.class;
		else if (context == "onCellsEvent")
			c = CellsEvent.class;
		else if (context == "onSubmitEvent")
			c = SubmitEvent.class;
		else
			return false;

		EventManager.unregisterAllEventListenersForContext(context);
		EventManager.registerEventListener(context, customizedListener, c);
		return true;

	}

	/* Default Handlers of the board (accessible to developers)
	 */
	
	/**
	 * Sets the alt flag to be true
	 * @param sender
	 * @param event
	 */
	public void defaultAltBtnHandler(Object sender, Event event) {
		board.setAltFlag(true);
	}
	
	/**
	 * Clears the submitted cell's information on the board
	 * @param sender
	 * @param event
	 */
	public void defaultSubmitHandler(Object sender, Event event) {
		SubmitEvent e = (SubmitEvent) event;
		board.setBitsAtCell(e.getCellInd(), 0);
		Log.i("EventTriggering", "Calling default onSubmit event handler");
	}


	/* Creates the default event Listeners set up for BWT
	 */
	
	/**
	 * Creates EventListener for touching the AltBtn
	 * @return
	 */
	private GenericEventListener createOnAltBtnListener() {
		return new GenericEventListener() {
			@Override
			public void eventTriggered(Object sender, Event event) {
				defaultAltBtnHandler(sender, event);
			}
		};
	}
	
	/**
	 * Creates EventListener for a submit event
	 * @return
	 */
	private GenericEventListener createOnSubmitListener() {
		return new GenericEventListener() {
			@Override
			public void eventTriggered(Object sender, Event event) {
				defaultSubmitHandler(sender, event);
			}
		};
	}

}

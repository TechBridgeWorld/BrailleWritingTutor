package edu.cmu.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;

import org.slf4j.Logger;

import edu.cmu.logger.EmulatorLogger;
//import edu.cmu.scripting.ScriptLoader;
import edu.cmu.server.JettyServer;

/**
 * A SWING GUI that gives the user the basic control over the embedded jetty
 * server. This should be the only entry point of this program. The web-based
 * emulator and http handlers should not be exposed to the user except from this
 * UI.
 * 
 * @author ziw
 * 
 */
public class ControllerUI extends JFrame {
	
	private static final long serialVersionUID = -583606563274987091L;
	
	private Logger logger = EmulatorLogger.getServerLogger();
	private UIActionHandler handler;

	private static final String WINDOW_LABEL = "BWT Emulator Control";

	// button labels
	public static final String START_BUTTON = "Start Server";
	public static final String STOP_BUTTON = "Stop Server";
	public static final String QUIT_BUTTON = "Quit";
	public static final String LAUNCH = "Launch";

	// buttons
	private JButton quitButton;
	private JButton startButton;
	private JButton stopButton;
	private JButton launchButton;

	// other components
	private JTextArea statusArea;
	private JTextArea msgArea;
	
	//tooltips
	private static final String START_SERVER_TOOLTIP = "Start the server.";
	private static final String STOP_SERVER_TOOLTIP = "Stop the server.";
	private static final String QUIT_TOOLTIP = "Stop the server and quit the program.";
	private static final String LAUNCH_TOOLTIP_ON = "Start the server and launch the emulator.";
	//private static final String LAUNCH_TOOLTIP_OFF = "Can't launch the emulator. Server is offline.";
		
	public ControllerUI() {
		initUI();
		try {
			handler = new UIActionHandler();
			updateStatusArea(getServerStatus());
		} catch (Exception e) {
			logger.error("Error occured when intializing the server.");
			EmulatorLogger.logException(logger, e);
			updateStatusArea("Error occured when initializing the server. Please check the server log.");
		}
	}
	


	private void initUI() {
		JPanel panel = new JPanel();
		getContentPane().add(panel);
		panel.setLayout(null);

		
		initLayout();
		bindActionListener();
		
		panel.add(statusArea);
		panel.add(quitButton);
		panel.add(launchButton);

		setTitle(WINDOW_LABEL);
		setSize(400, 200);
		setLocationRelativeTo(null);
		setDefaultCloseOperation(EXIT_ON_CLOSE);
	}

	private void updateStatusArea(String text) {
		statusArea.setText(text);
	}

	private void updateMsgArea(String text) {
		msgArea.setText(text);
	}
	
	private void initLayout(){
		quitButton = new JButton(QUIT_BUTTON);
		launchButton = new JButton(LAUNCH);

		launchButton.setBounds(20, 30, 120, 50);
		quitButton.setBounds(20, 100, 120, 50);

		statusArea = new JTextArea();
		statusArea.setEditable(false);
		statusArea.setBounds(220, 30, 150, 50);

		launchButton.setEnabled(true);
		
		quitButton.setToolTipText(QUIT_TOOLTIP);
		launchButton.setToolTipText(LAUNCH_TOOLTIP_ON);
		
		
	}

	private void bindActionListener() {

		quitButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				//try to stop the server before quit the program.
				try {
					handler.stopServer();
				} catch (Exception e1) {
					logger.error("Error when stopping the server when quitButton is clicked");
					EmulatorLogger.logException(logger, e1);
				} finally {
					System.exit(0);
				}
			}
		});


		launchButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {				
				try {
					handler.startServer();
				} catch (Exception e1) {
					logger.error("Exception when starting the server.");
					EmulatorLogger.logException(logger, e1);
				}
				boolean on = handler.isServerRunning();
				if(on){
					String url = "http://localhost:" + handler.getPort()
							+ "/index.html";
					try {
						java.awt.Desktop.getDesktop().browse(
								java.net.URI.create(url));
					} catch (IOException e1) {
						updateMsgArea("Error launching the emulator.");
						logger.error("Error opening the browser.");
						EmulatorLogger.logException(logger, e1);
					}
				}
				updateStatusArea(getServerStatus());

			}
		});
	}

	private String getServerStatus() {
		int status = handler.getServerStatus();
		int port = handler.getPort();
		String serverStatus;

		switch (status) {
			case JettyServer.RUNNING:
				serverStatus = "Running";
				break;
			case JettyServer.OFFLINE:
				serverStatus = "Offline";
				break;
			case JettyServer.ERROR:
			default:
				serverStatus = "Error. Unknown status";
		}

		StringBuffer sb = new StringBuffer();
		sb.append("Server Status: " + serverStatus + "\n");

		if (status == JettyServer.RUNNING)
			sb.append("Port:          " + port + "\n");
		return sb.toString();
	}

	public static void main(String[] args) {
		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				ControllerUI sg = new ControllerUI();
				sg.setVisible(true);
			}
		});
	}
}

package edu.cmu.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;

public class ControllerUI extends JFrame {

	private static final long serialVersionUID = -583606563274987091L;
	private UIActionHandler handler = new UIActionHandler();
	private UIActionListener actionListener = new UIActionListener();
	
	//button labels
	private static final String START_BUTTON = "Start Server";
	private static final String STOP_BUTTON = "Stop Server";
	private static final String QUIT_BUTTON = "Quit";
	private static final String LAUNCH = "Launch Emulator";
	
	
	//buttons
	private JButton quitButton;
	private JButton startButton;
	private JButton stopButton;
	private JButton launchButton;
	
	//other components
	private JTextArea statusArea;
	
	public ControllerUI(){
		initUI();
	}
	
	
	
	public void initUI() {
		JPanel panel = new JPanel();
		getContentPane().add(panel);

		panel.setLayout(null);

		quitButton = new JButton(QUIT_BUTTON);
		startButton  = new JButton(START_BUTTON);
		stopButton = new JButton(STOP_BUTTON);
		launchButton = new JButton(LAUNCH);
		
		
		launchButton.setBounds(100,200,120,50);
		quitButton.setBounds(100, 270, 120, 50);
		startButton.setBounds(100,60,120,50);
		stopButton.setBounds(100,130,120,50);
		
		statusArea = new JTextArea("this is a jtext area\nport: 8888");
		statusArea.setEditable(false);
		statusArea.setBounds(300, 60, 200, 30);
		
		quitButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent event) {
				System.exit(0);
			}
		});
		
		startButton.addActionListener(new ActionListener() {
			
			@Override
			public void actionPerformed(ActionEvent e) {
			}
		});
		
		stopButton.addActionListener(new ActionListener() {
			
			@Override
			public void actionPerformed(ActionEvent e) {
				Object o = e.getSource();

//				ac.stopServer();
			}
		});
		
		
		panel.add(statusArea);
		panel.add(quitButton);
		panel.add(startButton);
		panel.add(stopButton);
		panel.add(launchButton);
		
		
		setTitle("wahoo");
		setSize(600, 400);
		setLocationRelativeTo(null);
		setDefaultCloseOperation(EXIT_ON_CLOSE);
	}
	
	private void updateServerStatus(){
//		String status = ac.getServerStatus();
//		statusArea.setText(status);
	}

//	public static void main(String[] args) {
//		SwingUtilities.invokeLater(new Runnable() {
//			public void run() {
//				ControllerUI sg = new ControllerUI();
//				sg.setVisible(true);
//			}
//		});
//	}
}

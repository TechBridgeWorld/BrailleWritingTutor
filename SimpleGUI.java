import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;

import java.io.FileDescriptor;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Auto Generated Java Class.
 */
public class SimpleGUI extends JFrame {
  
  /* ADD YOUR CODE HERE */
  public SimpleGUI() {
    initUI();
  }
  
  public final void initUI() {
    JPanel panel = new JPanel();
    getContentPane().add(panel);
    
    panel.setLayout(null);
    
    JButton quitButton = new JButton("Quit");
    quitButton.setBounds(50, 60, 80, 30);
    quitButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent event) {
        System.exit(0);
      }
    });
    panel.add(quitButton);
    setTitle("wahoo");
    setSize(600, 400);
    setLocationRelativeTo(null);
    setDefaultCloseOperation(EXIT_ON_CLOSE);
  }
  
  public static void main(String[] args) {
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        SimpleGUI sg = new SimpleGUI();
        sg.setVisible(true);
      }
    });
  }
}

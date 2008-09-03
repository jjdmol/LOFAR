import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import ptolemy.plot.*;
import java.io.*;
import java.util.Properties;


public class lofarRadioSampler extends JPanel
                               implements ActionListener {

    class WaitForFileThread extends Thread {
	public void run () {
	    waitReadyFiles(this);
	    if (! removedFileThread.isAlive()) {
		System.out.println("restarting removedfilethread");
		removedFileThread = new WaitRemovedFileThread();
		removedFileThread.start();
	    }
	}
    }

    class WaitRemovedFileThread extends Thread {
	public void run () {
	    waitRemovedFiles(this);
	    if (! waitFileThread.isAlive() ) {
		System.out.println("restarting waitfilethread");
		waitFileThread = new WaitForFileThread();
		waitFileThread.start();
	    }
	    PMake();
	}
    }


    void buildConstraints(GridBagConstraints gbc, int gx, int gy, 
                          int gw, int gh, int wx, int wy) {
       gbc.gridx      = gx;
       gbc.gridy      = gy;
       gbc.gridwidth  = gw;
       gbc.gridheight = gh;
       gbc.weightx    = wx;
       gbc.weighty    = wy;
    }


    WaitForFileThread      waitFileThread;
    WaitRemovedFileThread  removedFileThread;
    Runtime                thisSystem;
    String                 workDir;
    String                 waveDir;
    boolean                isSampling          = false;
    JLabel                 lineLabel1;
    JLabel                 lineLabel2;
    JLabel                 lineLabel3;
    JLabel                 lineLabel4;
    JButton                endButton;
    JButton		   startButton;
    ImageIcon              activeBeepIcon;
    ImageIcon              silentBeepIcon;
    Plot		   plotWindow;


    public lofarRadioSampler() {

      // gather all images
      ImageIcon lofarIcon      = new ImageIcon("images/lofar.gif");
      ImageIcon antennaIcon    = new ImageIcon("images/antenna-small.gif");
                activeBeepIcon = new ImageIcon("images/beep.gif");
                silentBeepIcon = new ImageIcon("images/silentbeep.gif"); 
      ImageIcon pcIcon         = new ImageIcon("images/pc.gif");
      ImageIcon clusterIcon    = new ImageIcon("images/cluster.gif");

      // Initialise all labels
                lineLabel1     = new JLabel(silentBeepIcon);
                lineLabel2     = new JLabel(silentBeepIcon);
                lineLabel3     = new JLabel(silentBeepIcon);
                lineLabel4     = new JLabel(silentBeepIcon);
      JLabel    lofarLabel     = new JLabel(lofarIcon);
      JLabel    antennaLabel1  = new JLabel(antennaIcon);
      JLabel    antennaLabel2  = new JLabel(antennaIcon);
      JLabel    pcLabel1       = new JLabel(pcIcon);
      JLabel    pcLabel2       = new JLabel(pcIcon);
      JLabel    clusterLabel   = new JLabel(clusterIcon);	
      JLabel    filler1        = new JLabel();    
      JLabel    filler2        = new JLabel();    
      JLabel    filler3        = new JLabel();    
      JLabel    filler4        = new JLabel();    
      JLabel    filler5        = new JLabel();    

      // Try to find the workdirectory  (program has to be started 
      // with -DWORKDIR=$LOFARWORKDIR
      Properties properties = System.getProperties();
      workDir = System.getProperty("WORKDIR",".");
      waveDir = workDir+"/WAVE";
      System.out.println("workDir  found: "+workDir);
      System.out.println("waveDir  found: "+waveDir);

      // take system this sampler runs on
      thisSystem = Runtime.getRuntime();

      setBorder(BorderFactory. createCompoundBorder(
	        BorderFactory.createTitledBorder(
		"Lofar Radio Sampler"),
	        BorderFactory.createEmptyBorder(10,10,10,10)));
      setFont(new Font("Helvetica", Font.PLAIN, 14));

      GridBagLayout gridbag = new GridBagLayout();
      GridBagConstraints constraints = new GridBagConstraints();
      setLayout(gridbag);

      // Filler Label
      buildConstraints(constraints,0,0,1,1,100,100);
      gridbag.setConstraints(filler1,constraints);
      add(filler1);

      // Lofar Symbol
      buildConstraints(constraints,1,0,1,1,100,100);
      gridbag.setConstraints(lofarLabel,constraints);
      add(lofarLabel);

      // Filler Label
      buildConstraints(constraints,2,0,1,1,100,100);
      gridbag.setConstraints(filler2,constraints);
      add(filler2);

      // plot Window
      plotWindow = new Plot();
      buildConstraints(constraints,3,0,2,2,100,100);
      gridbag.setConstraints(plotWindow,constraints);
      add(plotWindow);

      // AntennaLabel1
      buildConstraints(constraints,0,1,1,1,100,100);
      gridbag.setConstraints(antennaLabel1,constraints);
      add(antennaLabel1);

      // Filler Label
      buildConstraints(constraints,1,1,2,1,100,100);
      gridbag.setConstraints(filler3,constraints);
      add(filler3);

      // Filler Label
      buildConstraints(constraints,0,2,1,1,100,100);
      gridbag.setConstraints(filler4,constraints);
      add(filler4);

      JPanel linePanel1 = new JPanel();
      linePanel1.setLayout(new GridLayout(2,1));

      buildConstraints(constraints,1,2,1,1,100,100);
      gridbag.setConstraints(linePanel1,constraints);
      add(linePanel1);

      // LineLabel1 && 2
      linePanel1.add(lineLabel1);
      linePanel1.add(lineLabel2);

      // PC label
      buildConstraints(constraints,2,2,1,1,100,100);
      gridbag.setConstraints(pcLabel1,constraints);
      add(pcLabel1);

      JPanel linePanel2 = new JPanel();
      linePanel2.setLayout(new GridLayout(2,1));
      buildConstraints(constraints,3,2,1,1,100,100);
      gridbag.setConstraints(linePanel2,constraints);
      add(linePanel2);

      // LineLabel3
      linePanel2.add(lineLabel3);
      // LineLabel4
      linePanel2.add(lineLabel4);

      // clusterLabel
      buildConstraints(constraints,4,2,1,1,100,100);
      gridbag.setConstraints(clusterLabel,constraints);
      add(clusterLabel);

      // AntennaLabel2
      buildConstraints(constraints,0,3,1,1,100,100);
      gridbag.setConstraints(antennaLabel2,constraints);
      add(antennaLabel2);

      // Filler Label
      buildConstraints(constraints,1,3,1,1,100,100);
      gridbag.setConstraints(filler5,constraints);
      add(filler5);

      // button group
      // start button
      startButton = new JButton("Sample");
      buildConstraints(constraints,2,3,1,1,100,100);
      gridbag.setConstraints(startButton,constraints);
      add(startButton);

      startButton.setMnemonic(KeyEvent.VK_S);
      startButton.setActionCommand("Sample");
      startButton.setMargin(new Insets(5,5,5,5));

      // Add Actionlistner
      startButton.addActionListener(this);
      startButton.setToolTipText("Click this button start "
				  + "taking radiosamples");

      // end button
      endButton = new JButton("End   ");
      buildConstraints(constraints,3,3,1,1,100,100);
      gridbag.setConstraints(endButton,constraints);
      add(endButton);

      endButton.setMnemonic(KeyEvent.VK_E);
      endButton.setActionCommand("End");

      // Add Actionlistner
      endButton.addActionListener(this);
      endButton.setToolTipText("Click this button to "
				+ "end taking radiosamples");
      endButton.setMargin(new Insets(5,5,5,5));
      endButton.setEnabled(false);

      // quit button
      JButton quitButton = new JButton("Quit  ");
      buildConstraints(constraints,4,3,1,1,100,100);
      gridbag.setConstraints(quitButton,constraints);
      add(quitButton);

      quitButton.setMnemonic(KeyEvent.VK_Q);
      quitButton.setActionCommand("Quit");

      // Add Actionlistner
      quitButton.addActionListener(this);
      quitButton.setToolTipText("Click this button to "
				 + "end program");
      quitButton.setMargin(new Insets(5,5,5,5));


      waitFileThread = new WaitForFileThread();
      removedFileThread = new WaitRemovedFileThread();

    }

    public void actionPerformed(java.awt.event.ActionEvent e) {
	if (e.getActionCommand().equals("Sample")) {
	  if (! isSampling) {
            // Call samplerscript and change icons
	    lineLabel1.setIcon(activeBeepIcon);
	    lineLabel2.setIcon(activeBeepIcon);
	    startButton.setEnabled(false);
	    endButton.setEnabled(true);
	    isSampling = true;
	    String prog="";
	    try {
	      prog=workDir+"/clean.csh";
	      thisSystem.exec(prog);
	      //prog=workDir+"/record.csh";
	      thisSystem.exec(prog);
	    } catch (IOException ex) {
              Message msg = new Message("Error in command: " + prog + " : " + ex);
	    }
	  }

	  if (! waitFileThread.isAlive()) {
	    System.out.println("restarting waitfilethread");
	    waitFileThread.start();
	  }
	} else if (e.getActionCommand().equals("End")) {
	  // end moving lines, remove touched files
	  String prog="";
	  try {
            prog=workDir+"/clean.csh";
	    thisSystem.exec(prog);
	  } catch (IOException ex) {
            Message msg = new Message("Error in command: " + prog + " : " + ex);
	  }
	  lineLabel1.setIcon(silentBeepIcon);
	  lineLabel2.setIcon(silentBeepIcon);
	  lineLabel3.setIcon(silentBeepIcon);
	  lineLabel4.setIcon(silentBeepIcon);
	  startButton.setEnabled(true);
	  endButton.setEnabled(false);
	  isSampling = false;
	} else if (e.getActionCommand().equals("Quit")) {
	    // quit program
	    System.exit(0);
	}
    }

    public void waitReadyFiles(WaitForFileThread aThread) {
	// wait until the two files: file1.ready file2.ready  are available

	boolean isReady = false;
        while (! isReady) {
	  File file1 = new File(waveDir,"antenna1.wav.ready");
	  if (file1.exists()) {
	    isReady = true;
	    isSampling = false;
	    lineLabel1.setIcon(silentBeepIcon);
	    lineLabel2.setIcon(silentBeepIcon);
	    lineLabel3.setIcon(activeBeepIcon);
	    lineLabel4.setIcon(activeBeepIcon);
	  }
	  try {
	    aThread.sleep(100);
	  } catch (InterruptedException ex) {
            Message msg = new Message("Thread allready Interrupted " + ex);
	  }
       }
    }
    

    public void waitRemovedFiles(WaitRemovedFileThread aThread) {
	// wait until the two files: file1.ready file2.ready  are removed

	boolean isReady = false;
        while (! isReady) {
	  File file1 = new File(waveDir,"antenna1.wav.ready");
	  if (! file1.exists()) {
	    isReady = true;
	    isSampling = true;
	    endButton.setEnabled(true);
	    lineLabel1.setIcon(activeBeepIcon);
	    lineLabel2.setIcon(activeBeepIcon);
	    lineLabel3.setIcon(silentBeepIcon);
	    lineLabel4.setIcon(silentBeepIcon);
	    String prog="";
	    try {
		prog=workDir+"/clean.csh";
		thisSystem.exec(prog);
		//prog=workDir+"/record.csh";
		thisSystem.exec(prog);
	    } catch (IOException ex) {
              Message msg = new Message("Error in command: " + prog + " : " + ex);
	    }
	  }
	  try {
	      aThread.sleep(100);
	  } catch (InterruptedException ex) {
            Message msg = new Message("Thread allready Interrupted " + ex);
	  }
       }
    }      

    public void PMake() {
      File file = new File(waveDir,"Corr.ext");
      try {
        plotWindow.clear(true);
	plotWindow.read(new FileInputStream(file));
      } catch (FileNotFoundException ex) {
        Message msg = new Message("File not found: " + file + " : " + ex);
      } catch (IOException ex) {
        Message msg = new Message("Error reading input: " + file +
				  " : " + ex);
      }
	    
      // init plotter control
      plotWindow.setYRange(-0.5,1);
      plotWindow.setButtons(false);
      plotWindow.setTitle("Lofar plot");
      plotWindow.setYLabel("Correlatie Coefficient");
      plotWindow.repaint();
    }


    public static void main(String args[]) {
        JFrame mainFrame = new JFrame("lofarRadioSampler");

        mainFrame.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                System.exit(0);
            }
        });

        mainFrame.getContentPane().add(new lofarRadioSampler());
        mainFrame.pack();
        mainFrame.setVisible(true);
    }
}
 





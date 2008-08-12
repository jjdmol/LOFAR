/*
 * tfc_gui.java
 *
 * Created on October 14, 2005, 10:57 AMa
 */

package tfc_gui;
import java.awt.Image;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import javax.imageio.ImageIO;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;

/**
 *
 * @author  coolen
 */
public class tfc_gui extends javax.swing.JFrame {
    
    private String       itsSelectedFile="None";
    private Image        image = null;
    private ImageDisplay id;
    private Runtime      thisSystem;
    private String       runCommands[]={"bgl","storage","wrapper","delay","input"};
    private String       runCmdReady[]={"PPF_And_Correlator","Storage","SocketWrapper","DelayCompensation","InputSection"};
    private String       ppCommands[]={"postproc"};
    private String       ppCmdReady[]={"PostProcessing"};
    private boolean      singleProcess=false;
    
    private String workDir="";
    private File itsConfigurationFile=null;
 
    class WaitForFileThread extends Thread {
        String theJobName;
        
        public void setJobTerminationName(String aJobName) {
            theJobName=aJobName;
        }
        
	public void run () {
	    waitReadyFiles(this,theJobName);
	}
    }


    /** Creates new form tfc_gui */
    public tfc_gui() {
        try {
            workDir=new File(".").getCanonicalPath();
            // take system this Demo runs on
            thisSystem = Runtime.getRuntime();
            initComponents();
            printLog("Workdir: "+workDir);
            LogPaneTextArea.setEditable(false);
            
            // Try to load in the initial file if available
            File aFile= new File("TFlopCorrelator.cfg");
            if (aFile.exists() && aFile.canRead() && aFile.canWrite()) {
                if (loadFile(aFile)) {
                    itsConfigurationFile=aFile;
                }
            }
        } catch(Exception e) {
            printLog("Error getting running system");
            e.printStackTrace();
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        ButtonPanel = new javax.swing.JPanel();
        StartButton = new javax.swing.JButton();
        PlotComboBox = new javax.swing.JComboBox();
        StartPlotButton = new javax.swing.JButton();
        clearButton = new javax.swing.JButton();
        StopAllButton = new javax.swing.JButton();
        StartBGLButton = new javax.swing.JButton();
        StartWrapperButton = new javax.swing.JButton();
        StartStorageButton = new javax.swing.JButton();
        StartDelayButton = new javax.swing.JButton();
        StartInputButton = new javax.swing.JButton();
        StopWrapperButton = new javax.swing.JButton();
        StartPostProcessingButton = new javax.swing.JButton();
        tfc_guiTabbedPane = new javax.swing.JTabbedPane();
        ConfigPanePanel = new javax.swing.JPanel();
        ConfigPane = new javax.swing.JScrollPane();
        ConfigPaneTextArea = new javax.swing.JTextArea();
        saveConfigButton = new javax.swing.JButton();
        LogPane = new javax.swing.JScrollPane();
        LogPaneTextArea = new javax.swing.JTextArea();
        PlotPane = new javax.swing.JScrollPane();
        tfc_guiMenuBar = new javax.swing.JMenuBar();
        FileMenu = new javax.swing.JMenu();
        FileMenuOpenFile = new javax.swing.JMenuItem();
        jSeparator1 = new javax.swing.JSeparator();
        FileMenuExit = new javax.swing.JMenuItem();
        SettingsMenu = new javax.swing.JMenu();
        SettingsMenuTestSetting = new javax.swing.JCheckBoxMenuItem();

        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        ButtonPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        StartButton.setText("Start TFlopCorrelator");
        StartButton.setToolTipText("Start all processes for the TFlopCorrelator");
        StartButton.setBorder(new javax.swing.border.BevelBorder(javax.swing.border.BevelBorder.RAISED));
        StartButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StartButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 19, 190, 30));

        PlotComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Auto and cross correlation", "Cross correlation", "Phase of cross correlation" }));
        ButtonPanel.add(PlotComboBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 300, 200, -1));

        StartPlotButton.setText("Show");
        StartPlotButton.setToolTipText("Show the selected plot");
        StartPlotButton.setBorder(new javax.swing.border.BevelBorder(javax.swing.border.BevelBorder.RAISED));
        StartPlotButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartPlotButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StartPlotButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 330, -1, -1));

        clearButton.setText("Clear All");
        clearButton.setToolTipText("Clear all GUI Screens");
        clearButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                clearButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(clearButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 410, 90, -1));

        StopAllButton.setText("Stop All");
        StopAllButton.setToolTipText("Stop All remote running programs");
        StopAllButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StopAllButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StopAllButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 380, 90, -1));

        StartBGLButton.setText("Start BGL");
        StartBGLButton.setToolTipText("Start PPF and Correlator");
        StartBGLButton.setEnabled(false);
        StartBGLButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartBGLButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StartBGLButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 60, -1, -1));

        StartWrapperButton.setText("Start Wrapper");
        StartWrapperButton.setToolTipText("Start Remote Socket Wrapper");
        StartWrapperButton.setEnabled(false);
        StartWrapperButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartWrapperButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StartWrapperButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 120, -1, -1));

        StartStorageButton.setText("Start Storage");
        StartStorageButton.setToolTipText("Start Remote Storage");
        StartStorageButton.setEnabled(false);
        StartStorageButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartStorageButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StartStorageButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 90, -1, -1));

        StartDelayButton.setText("Start Delay");
        StartDelayButton.setToolTipText("Start Remote DelayCompensation");
        StartDelayButton.setEnabled(false);
        StartDelayButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartDelayButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StartDelayButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 150, -1, -1));

        StartInputButton.setText("Start Input");
        StartInputButton.setToolTipText("Start Remote InputSection");
        StartInputButton.setEnabled(false);
        StartInputButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartInputButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StartInputButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 180, -1, -1));

        StopWrapperButton.setText("Stop Wrapper");
        StopWrapperButton.setToolTipText("Stop Remote SocketWrapper");
        StopWrapperButton.setEnabled(false);
        StopWrapperButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StopWrapperButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StopWrapperButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 210, -1, -1));

        StartPostProcessingButton.setText("Start Post Processing");
        StartPostProcessingButton.setToolTipText("Start Remote PostProcessing");
        StartPostProcessingButton.setEnabled(false);
        StartPostProcessingButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StartPostProcessingButtonActionPerformed(evt);
            }
        });

        ButtonPanel.add(StartPostProcessingButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 240, -1, -1));

        getContentPane().add(ButtonPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, 210, 580));

        ConfigPanePanel.setLayout(new java.awt.BorderLayout());

        ConfigPane.setViewportView(ConfigPaneTextArea);

        ConfigPanePanel.add(ConfigPane, java.awt.BorderLayout.CENTER);

        saveConfigButton.setText("Save");
        saveConfigButton.setToolTipText("save the configuration");
        saveConfigButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveConfigButtonActionPerformed(evt);
            }
        });

        ConfigPanePanel.add(saveConfigButton, java.awt.BorderLayout.SOUTH);

        tfc_guiTabbedPane.addTab("Config", ConfigPanePanel);

        LogPane.setViewportView(LogPaneTextArea);

        tfc_guiTabbedPane.addTab("Log", LogPane);

        tfc_guiTabbedPane.addTab("Plot", PlotPane);

        getContentPane().add(tfc_guiTabbedPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(220, 20, 680, 520));

        FileMenu.setText("File");
        FileMenuOpenFile.setText("Open Configuration File");
        FileMenuOpenFile.setToolTipText("Select configuration file to be used");
        FileMenuOpenFile.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                FileMenuOpenFileActionPerformed(evt);
            }
        });

        FileMenu.add(FileMenuOpenFile);

        FileMenu.add(jSeparator1);

        FileMenuExit.setText("Exit");
        FileMenuExit.setToolTipText("Stop Program");
        FileMenuExit.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                FileMenuExitActionPerformed(evt);
            }
        });

        FileMenu.add(FileMenuExit);

        tfc_guiMenuBar.add(FileMenu);

        SettingsMenu.setText("Settings");
        SettingsMenu.setToolTipText("Settings Menu");
        SettingsMenuTestSetting.setText("Test");
        SettingsMenuTestSetting.setToolTipText("Set TestStage");
        SettingsMenuTestSetting.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SettingsMenuTestSettingActionPerformed(evt);
            }
        });

        SettingsMenu.add(SettingsMenuTestSetting);

        tfc_guiMenuBar.add(SettingsMenu);

        setJMenuBar(tfc_guiMenuBar);

        pack();
    }
    // </editor-fold>//GEN-END:initComponents

    private void saveConfigButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveConfigButtonActionPerformed
        if (itsConfigurationFile != null) {
            saveFile();
        }
    }//GEN-LAST:event_saveConfigButtonActionPerformed

    private void StartPostProcessingButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartPostProcessingButtonActionPerformed
        startPostProcessing();
    }//GEN-LAST:event_StartPostProcessingButtonActionPerformed

    private void StopWrapperButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StopWrapperButtonActionPerformed
        stopWrapper();
    }//GEN-LAST:event_StopWrapperButtonActionPerformed

    private void StartInputButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartInputButtonActionPerformed
        startSingleProcess(4);
    }//GEN-LAST:event_StartInputButtonActionPerformed

    private void StartDelayButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartDelayButtonActionPerformed
        startSingleProcess(3);
    }//GEN-LAST:event_StartDelayButtonActionPerformed

    private void StartWrapperButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartWrapperButtonActionPerformed
        startSingleProcess(2);
    }//GEN-LAST:event_StartWrapperButtonActionPerformed

    private void StartBGLButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartBGLButtonActionPerformed
        startSingleProcess(0);
    }//GEN-LAST:event_StartBGLButtonActionPerformed

    private void StopAllButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StopAllButtonActionPerformed
        stopAllRemote();
    }//GEN-LAST:event_StopAllButtonActionPerformed

    private void SettingsMenuTestSettingActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SettingsMenuTestSettingActionPerformed
        activateTestButtons(SettingsMenuTestSetting.isSelected());
    }//GEN-LAST:event_SettingsMenuTestSettingActionPerformed

    private void StartStorageButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartStorageButtonActionPerformed
       startSingleProcess(1);
    }//GEN-LAST:event_StartStorageButtonActionPerformed

    private void clearButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_clearButtonActionPerformed
        clearAll();
    }//GEN-LAST:event_clearButtonActionPerformed

    private void StartButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartButtonActionPerformed
        tfc_guiTabbedPane.setSelectedIndex(1);
        startPipeline();
    }//GEN-LAST:event_StartButtonActionPerformed

    private void StartPlotButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StartPlotButtonActionPerformed
        tfc_guiTabbedPane.setSelectedIndex(2);
        displayPlot();
    }//GEN-LAST:event_StartPlotButtonActionPerformed

    private void FileMenuExitActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FileMenuExitActionPerformed
        System.exit(0);
    }//GEN-LAST:event_FileMenuExitActionPerformed

    private void FileMenuOpenFileActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FileMenuOpenFileActionPerformed
        tfc_guiTabbedPane.setSelectedIndex(0);
        readFile();
    }//GEN-LAST:event_FileMenuOpenFileActionPerformed
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new tfc_gui().setVisible(true);
            }
        });
    }
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel ButtonPanel;
    private javax.swing.JScrollPane ConfigPane;
    private javax.swing.JPanel ConfigPanePanel;
    private javax.swing.JTextArea ConfigPaneTextArea;
    private javax.swing.JMenu FileMenu;
    private javax.swing.JMenuItem FileMenuExit;
    private javax.swing.JMenuItem FileMenuOpenFile;
    private javax.swing.JScrollPane LogPane;
    private javax.swing.JTextArea LogPaneTextArea;
    private javax.swing.JComboBox PlotComboBox;
    private javax.swing.JScrollPane PlotPane;
    private javax.swing.JMenu SettingsMenu;
    private javax.swing.JCheckBoxMenuItem SettingsMenuTestSetting;
    private javax.swing.JButton StartBGLButton;
    private javax.swing.JButton StartButton;
    private javax.swing.JButton StartDelayButton;
    private javax.swing.JButton StartInputButton;
    private javax.swing.JButton StartPlotButton;
    private javax.swing.JButton StartPostProcessingButton;
    private javax.swing.JButton StartStorageButton;
    private javax.swing.JButton StartWrapperButton;
    private javax.swing.JButton StopAllButton;
    private javax.swing.JButton StopWrapperButton;
    private javax.swing.JButton clearButton;
    private javax.swing.JSeparator jSeparator1;
    private javax.swing.JButton saveConfigButton;
    private javax.swing.JMenuBar tfc_guiMenuBar;
    private javax.swing.JTabbedPane tfc_guiTabbedPane;
    // End of variables declaration//GEN-END:variables
 
    private void readFile() {
        itsSelectedFile = "None";
        JFileChooser fc = new JFileChooser();
        int returnVal = fc.showOpenDialog(tfc_gui.this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            File aFile = fc.getSelectedFile();
            itsSelectedFile=aFile.getName();
            if (loadFile(aFile)) {
                itsConfigurationFile=aFile;
            }
        } else {
              JOptionPane.showMessageDialog(null,"You didn't select a file",
                      "Config selection warning",
                      JOptionPane.WARNING_MESSAGE);
        }
    }
    
    private void saveFile() {
        try {
            BufferedWriter aW=new BufferedWriter(new FileWriter(itsConfigurationFile));
            ConfigPaneTextArea.write(aW);
            aW.close();
        } catch (IOException e) {
            printLog("Error writing to file: "+itsConfigurationFile.getName());
            e.printStackTrace();
        }
        
        JOptionPane.showMessageDialog(null,"New ConfigurationFile Saved!!",
                "File Saved Message",
                JOptionPane.INFORMATION_MESSAGE);
    }
    
    private boolean loadFile(File aFile) {
        try {
            BufferedReader in = new BufferedReader(new FileReader(aFile));
            String aStr;
            while ((aStr = in.readLine()) != null) {
                ConfigPaneTextArea.append(aStr+"\n");
            }
            in.close();                     
        } catch (IOException e) {
            printLog("Error reading from file: "+aFile.getName());
            e.printStackTrace();
            return false;
        } 
        return true;
    }
    
    private void displayPlot() {
        
         File aFile=new File(workDir+"/plot"+(PlotComboBox.getSelectedIndex()+1)+".png");
         try {
            id = new ImageDisplay(ImageIO.read(aFile));
         } catch (IOException e) {
            printLog("Error reading from file: "+aFile.getName());
            e.printStackTrace();
         }
         PlotPane.setViewportView(id);
    }
    private void startPipeline() {
        removeReadyFiles();
        printLog("\n------------------ Starting the Pipeline ------------------\n");
        startProcess(runCommands,runCmdReady);
    }
    
    private void startProcess(String[] run,String[] ready) {
        String prog="";
        boolean cont=true;
        for (int i=0;i < run.length;i++) {
            prog=workDir+"/startTFC.sh "+run[i];
            if (cont) {
                printLog("Starting "+ready[i]);
                cont=startRemote(prog);
                WaitForFileThread aT=new WaitForFileThread();
                aT.setJobTerminationName(ready[i]);
                aT.start();
            } else {
                printLog("Error detected, skipping "+prog);
            }
        }           
    }
    
     private void startSingleProcess(int i) {
        removeReadyFile(runCmdReady[i]+".ready");
        String prog="";
        boolean cont=true;
        tfc_guiTabbedPane.setSelectedIndex(1);
        prog=workDir+"/startTFC.sh "+runCommands[i];
        printLog("Starting "+runCmdReady[i]);
        cont=startRemote(prog);
        if (cont) {
            WaitForFileThread aT=new WaitForFileThread();
            aT.setJobTerminationName(runCmdReady[i]);
            aT.start();
        } else {
            printLog("Error detected. "+prog+" NOT started");
        }
    }
     
    private boolean startRemote(String aRemoteCommand) {
        try {
	    thisSystem.exec(aRemoteCommand);
	} catch (IOException ex) {
            printLog("Error in command: " + aRemoteCommand + " : " + ex);
            return false;
	}       
        return true;
    }
    
    private void printLog(String aString) {
        LogPaneTextArea.append(aString+"\n");
    }
    
   
    public void waitReadyFiles(WaitForFileThread aThread,String aJobName) {
        // wait until the file aJobName.ready is available
        boolean isReady = false;
        while (! isReady) {
            File file1 = new File(workDir,aJobName+".ready");
            if (file1.exists()) {
                printLog(aJobName+" finished");
                isReady=true;
                if (aJobName.equals("Storage") && !singleProcess) {
                    stopWrapper();
                    startPostProcessing();
                } else if (aJobName.equals("PostProcessing")){
                    printLog("Plots should be ready");
                } 
                file1.delete();
            }
            try {
                aThread.sleep(100);
            } catch (InterruptedException ex) {
                printLog("Thread allready Interrupted " + ex);
            }
        }
    }
    
    private void clearAll() {
        ConfigPaneTextArea.setText("");
        LogPaneTextArea.setText("");
        PlotPane.setViewportView(null);
        itsSelectedFile="None";
    }
    
    private void removeReadyFile(String aName) {
        File aFile = new File(workDir,aName);
        if (aFile.exists()) {
            aFile.delete();
        }        
    }
    
    private void removeReadyFiles() {
        // Remove all .ready files from workDir
        int i;
        
        printLog("Removing all left .readyFiles");
        
        for (i=0; i< runCmdReady.length; i++) {
            removeReadyFile(runCmdReady[i]+".ready");
        }
        for (i=0; i< ppCmdReady.length; i++) {
            removeReadyFile(ppCmdReady[i]+".ready");
        }
    }
    
    private void startPostProcessing() {
        removeReadyFile("PostProcessing.ready");
        printLog("\n------------------ Starting the PostProcessing ------------------\n");
        startProcess(ppCommands,ppCmdReady);
    }
    
    private void stopWrapper() {
        removeReadyFile("SocketWrapper.ready");
        printLog("\n------------------ Stopping the Wrapper ------------------\n");
        String prog=workDir+"/startTFC.sh wrapper stop";
        startRemote(prog);
    }
    
    private void activateTestButtons(Boolean aFlag) {
        StartBGLButton.setEnabled(aFlag);
        StartWrapperButton.setEnabled(aFlag);
        StartStorageButton.setEnabled(aFlag);
        StartDelayButton.setEnabled(aFlag);
        StartInputButton.setEnabled(aFlag);
        StopWrapperButton.setEnabled(aFlag);
        StartPostProcessingButton.setEnabled(aFlag);
        StartButton.setEnabled(!aFlag);
        singleProcess=aFlag;
    }
    
    private void stopAllRemote() {
        printLog("\n----------- Trying to stop all remote Programs -----------\n");
        String prog=workDir+"/stopAll.sh";
        startRemote(prog);
    }

}
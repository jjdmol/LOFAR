package org.astron.util.gui;

import java.awt.*;
import java.awt.event.*;
//import com.sun.java.swing.*;
import javax.swing.*;

public class Test extends JFrame
{
 	String[] data = {"This text is partially obscured by the JScrollPaneEx", "one blablablabla dddddddddd", "two blabla ajshdflkajsdhf", "free", "four"};
	JListEx list = new JListEx(data);
    //ImageIcon image = new ImageIcon("e:\\Projects\\SwingEx\\codeguruwm.gif");
    ImageIcon image = new ImageIcon("e:\\Projects\\SwingEx\\hiero.jpg");
    JButton BnTest = new JButton("Test");

	public Test()
	{
		try
		{
			jbInit();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}

	public static void main(String[] args)
	{
		Test test = new Test();
        test.pack();
        test.setVisible(true);
	}

	private void jbInit() throws Exception
	{
    	BnTest.setToolTipText("Test Button");
    	list.setDataTips(true);
        list.setBackgroundImage(image);
        list.setScrollableBackground(false);
    	getContentPane().setLayout(new BorderLayout());
        getContentPane().add(list, BorderLayout.CENTER);
        getContentPane().add(BnTest, BorderLayout.SOUTH);
        this.setTitle("SwingEx Test");
		this.addWindowListener(new java.awt.event.WindowAdapter()
		{
			public void windowClosing(WindowEvent e)
			{
				this_windowClosing(e);
			}
		});
	}

	void this_windowClosing(WindowEvent e)
	{
    	setVisible(false);
        dispose();
        System.exit(0);
	}
}

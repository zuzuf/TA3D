import java.applet.Applet;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JEditorPane;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.ListSelectionModel;
import javax.swing.Timer;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;


public class Main
	extends Applet
	implements ActionListener, ListSelectionListener {

	/**
	 * 
	 */
	private static final long serialVersionUID = 301780964753797516L;

	private JEditorPane tLogs;
	private JTextField tInput;
	private NetClient netClient;
	private StringBuilder logs;
	private InputDialog loginDialog;
	private InputDialog registerDialog;
	private JList userList;
	private JList chanList;
	private DefaultListModel userListData;
	private DefaultListModel chanListData;
	
	private static final String enterCommand = "TINPUT_ENTER";
	private static final String timerCommand = "TIMER_TIMEOUT";
	private static final String loginCommand = "LOGIN";
	private static final String logoutCommand = "LOGOUT";
	private static final String registerCommand = "REGISTER";
	
	public void init()
	{
		loginDialog = new InputDialog("login");
		registerDialog = new InputDialog("register");
		
		netClient = new NetClient();
		logs = new StringBuilder();
		
		BoxLayout layout = new BoxLayout(this, BoxLayout.Y_AXIS);
		setLayout(layout);

		userListData = new DefaultListModel();
		userListData.addElement("---users---");
		userList = new JList(userListData);
		userList.setVisibleRowCount(5);
		userList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		userList.setBackground(new Color(200, 200, 200));

		chanListData = new DefaultListModel();
		chanListData.addElement("---chans---");
		chanList = new JList(chanListData);
		chanList.setVisibleRowCount(5);
		chanList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		chanList.setBackground(new Color(200, 200, 200));
		chanList.addListSelectionListener(this);
	
		JPanel pMain = new JPanel();
		pMain.setLayout(new BorderLayout());
		
		tLogs = new JEditorPane();
		tLogs.setEditable(false);
		tLogs.setContentType("text/html; charset=UTF-8");
		pMain.add(new JScrollPane(chanList), BorderLayout.WEST);
		pMain.add(new JScrollPane(tLogs), BorderLayout.CENTER);
		pMain.add(new JScrollPane(userList), BorderLayout.EAST);
		add(pMain);
		userList.setMinimumSize(new Dimension(80, 80));
		chanList.setMinimumSize(new Dimension(80, 80));

		JPanel inputPanel = new JPanel();
		add(inputPanel);
		BoxLayout inputLayout = new BoxLayout(inputPanel, BoxLayout.X_AXIS);
		inputPanel.setLayout(inputLayout);
		
		tInput = new JTextField();
		tInput.setMaximumSize(new Dimension(10000, 20));
		tInput.setMinimumSize(new Dimension(128, 20));
		tInput.setActionCommand(enterCommand);
		tInput.addActionListener(this);
		inputPanel.add(tInput);
		
		JButton bSend = new JButton("send");
		bSend.addActionListener(this);
	
		inputPanel.add(bSend);
		
		JPanel buttonPanel = new JPanel();
		add(buttonPanel);
		BoxLayout buttonLayout = new BoxLayout(buttonPanel, BoxLayout.X_AXIS);
		buttonPanel.setLayout(buttonLayout);
		
		JButton bLogin = new JButton("login");
		bLogin.setActionCommand(loginCommand);
		bLogin.addActionListener(this);
		buttonPanel.add(bLogin);

		JButton bRegister = new JButton("register");
		bRegister.setActionCommand(registerCommand);
		bRegister.addActionListener(this);
		buttonPanel.add(bRegister);

		JButton bLogout = new JButton("logout");
		bLogout.setActionCommand(logoutCommand);
		bLogout.addActionListener(this);
		buttonPanel.add(bLogout);
		
		Timer timer = new Timer(100, this);
		timer.setActionCommand(timerCommand);
		timer.start();
	}
	
	@Override
	public void actionPerformed(ActionEvent e) {
		if (timerCommand.equals(e.getActionCommand()))
		{
			String msg = netClient.pollMessage();
			if (msg != null)
			{
				logs.append(msg);
				logs.append("<br>");
				tLogs.setText("<html>" + logs.toString() + "</html>");
			}
			
			if (netClient.userListHasChanged())
			{
				userListData.clear();
				userListData.addElement("---users---");
				for(String user : netClient.getUserList())
					userListData.addElement(user);
				userList.setModel(userListData);
				repaint();
			}
			
			if (netClient.chanListHasChanged())
			{
				chanListData.clear();
				chanListData.addElement("---chans---");
				for(String chan : netClient.getChanList())
					chanListData.addElement(chan);
				chanList.setModel(chanListData);
				repaint();
			}
			
			// Try to log or register
			if (netClient.getState() == NetClient.CONNECTED)
			{
				String login = loginDialog.getLogin();
				String password = loginDialog.getPassword();
				if (login != null && password != null)
					netClient.login(login, password);
				
				login = registerDialog.getLogin();
				password = registerDialog.getPassword();
				if (login != null && password != null)
					netClient.register(login, password);
			}
		}
		else if (enterCommand.equals(e.getActionCommand()))
		{
			String command = tInput.getText();
			tInput.setText(new String());
			if (command.startsWith("/"))
			{
				if (command.startsWith("/CHAN"))
					netClient.changeChan(command.substring(5).trim());
				else
					netClient.send(command.substring(1));
			}
			else if (netClient.getState() == NetClient.LOGGED && command != null && !command.isEmpty())
			{
				netClient.send("SENDALL " + command);
				command = "<font color=\"#0000ff\">&lt;" + netClient.getLogin() + "&gt; " + command + "</font>";
			}
			else
				command = null;
			
			if (command != null)
			{
				logs.append(command);
				logs.append("<br>");
				tLogs.setText("<html><head></head><body>" + logs.toString() + "</body></html>");
			}
		}
		else if (loginCommand.equals(e.getActionCommand()))
		{
			if (netClient.getState() != NetClient.LOGGED)
			{
				netClient.connect();
				loginDialog.open();
			}
		}
		else if (logoutCommand.equals(e.getActionCommand()))
		{
			if (netClient.getState() == NetClient.LOGGED)
				netClient.logout();
		}
		else if (registerCommand.equals(e.getActionCommand()))
		{
			if (netClient.getState() != NetClient.LOGGED)
			{
				netClient.connect();
				registerDialog.open();
			}
		}
	}

	@Override
	public void valueChanged(ListSelectionEvent e) {
		if (e.getValueIsAdjusting() == false) {
			if (chanList.getSelectedIndex() >= 1) {
				String chan = (String)chanListData.get(chanList.getSelectedIndex());
				netClient.changeChan(chan);
				logs.append("<font color=\"#007000\">entering chan '" + chan + "'</font><br>");
				tLogs.setText("<html><head></head><body>" + logs.toString() + "</body></html>");
			}
		}
	}
}

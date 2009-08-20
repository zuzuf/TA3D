import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;


public class InputDialog extends Frame implements ActionListener {

	/**
	 * 
	 */
	private static final long serialVersionUID = 5249122201797373214L;

	private String login;
	private String password;
	private JTextField tLogin;
	private JTextField tPassword;
	
	public InputDialog(String title) {
		setTitle(title);
		login = null;
		password = null;
		
		BoxLayout layout = new BoxLayout(this, BoxLayout.Y_AXIS);
		setLayout(layout);
		
		JPanel lPanel = new JPanel();
		add(lPanel);
		BoxLayout lLayout = new BoxLayout(lPanel, BoxLayout.X_AXIS);
		lPanel.setLayout(lLayout);
		JLabel lLogin = new JLabel("login: ");
		lPanel.add(lLogin);
		tLogin = new JTextField();
		lPanel.add(tLogin);
		
		JPanel pPanel = new JPanel();
		add(pPanel);
		BoxLayout pLayout = new BoxLayout(pPanel, BoxLayout.X_AXIS);
		pPanel.setLayout(pLayout);
		JLabel lPassword = new JLabel("password: ");
		pPanel.add(lPassword);
		tPassword = new JTextField();
		pPanel.add(tPassword);
		
		JButton bok = new JButton("ok");
		bok.setActionCommand("ok");
		bok.addActionListener(this);
		add(bok);
		
		setSize(300, 100);
		setResizable(false);
	}
	
	public void open()
	{
		login = null;
		password = null;
		tLogin.setText("");
		tPassword.setText("");
		setVisible(true);
	}
	
	public void close()
	{
		setVisible(false);
	}
	
	public String getLogin()
	{
		String tmp = login;
		login = null;
		return tmp;
	}
	
	public String getPassword()
	{
		String tmp = password;
		password = null;
		return tmp;
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		if (e.getActionCommand().equals("ok"))
		{
			login = tLogin.getText();
			password = tPassword.getText();
			close();
			tLogin.setText("");
			tPassword.setText("");
		}
	}
}

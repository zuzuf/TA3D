import java.net.Socket;
import java.util.Collections;
import java.util.LinkedList;



public class NetClient {

	// global constants
	public static final int DISCONNECTED = 0x0;
	public static final int CONNECTED = 0x1;
	public static final int LOGGED = 0x2;

	// object data
	private Socket sock;
	private int state;
	private StringBuilder inbuf;
	private LinkedList<String> chanList;
	private LinkedList<String> userList;
	private String serverVersion;
	private String login;
	private boolean userListChanged;
	private boolean chanListChanged;
	
	public NetClient()
	{
		state = DISCONNECTED;
		connect();
	}
	
	public void connect()
	{
		userListChanged = true;
		chanListChanged = true;

		if (state == CONNECTED)
			return;
		chanList = new LinkedList<String>();
		userList = new LinkedList<String>();
		serverVersion = "";
		try
		{
			sock = new Socket("netserver.ta3d.org", 4240);
		} catch(Exception e)	{ sock = null; }
		if (sock == null)
			state = DISCONNECTED;
		else
			state = sock.isConnected() ? CONNECTED : DISCONNECTED;
		
		if (state == CONNECTED)
		{
			inbuf = new StringBuilder();
			send("CLIENT JNetClient");
			send("SET CHAT_MODE 1");
		}
		login = "";
	}
	
	public void send(String str)
	{
		if (str.isEmpty() || state == DISCONNECTED)	return;
		if (sock == null || !sock.isConnected())
		{
			state = DISCONNECTED;
			return;
		}

		try
		{
			if (!str.endsWith("\n"))
				str = str + "\n";
			sock.getOutputStream().write(str.getBytes("UTF-8"));
			sock.getOutputStream().flush();
		} catch(Exception e)
		{
			close();
		}
	}

	public void close()
	{
		userList.clear();
		chanList.clear();
		
		inbuf = new StringBuilder();
		state = DISCONNECTED;
		if (sock == null)
			return;
		try {
			sock.close();
		} catch(Exception e) {}
	}
	
	public int getState()
	{
		if (sock == null || !sock.isConnected())
			state = DISCONNECTED;
		return state;
	}
	
	public void changeChan(String chan)
	{
		userList.clear();
		userListChanged = true;
		send("CHAN " + chan);
		send("GET USER LIST");
	}
	
	public String pollMessage()
	{
		String msg = null;
		try {
			int limit = sock.getInputStream().available();
			for(int i = 0 ; i < limit ; ++i)
			{
				int c = sock.getInputStream().read();
				if (c >= 0 && c <= 255)
				{
					if ((char)c == '\n')
					{
						msg = inbuf.toString();
						inbuf.setLength(0);
						break;
					}
					else
						inbuf.append((char)c);
				}
			}
		} catch(Exception e)
		{
			close();
		}
		
		if (msg != null)
		{
			String args[] = msg.split(" ", 3);
			if (args.length == 0)	return null;
			if (args[0].equals("MESSAGE"))
			{
				msg = "<font color=\"#007000\">[server] " + msg.substring(8, msg.length()) + "</font>";
			}
			else if (args[0].equals("ERROR"))
			{
				msg = "<font color=\"#ff0000\">[error] " + msg.substring(6, msg.length()) + "</font>";
			}
			else if (args[0].equals("MSG"))
			{
				msg = "<font color=\"#0000ff\">&lt;" + args[1] + "&gt; " + args[2] + "</font>";
			}
			else if (args[0].equals("USER"))
			{
				addUser(args[1]);
				msg = null;
			}
			else if (args[0].equals("LEAVE"))
			{
				removeUser(args[1]);
				msg = null;
			}
			else if (args[0].equals("CHAN"))
			{
				addChan(args[1]);
				msg = null;
			}
			else if (args[0].equals("DECHAN"))
			{
				removeChan(args[1]);
				msg = null;
			}
			else if (args[0].equals("CLOSE"))
			{
				close();
				msg = "<font color=\"#007000\">[msg] You have been disconnected</font>";
			}
			else if (args[0].equals("CONNECTED"))
			{
				state = LOGGED;
				msg = "<font color=\"#007000\">[msg] You are now connected to NetServer " + serverVersion + "</font>";
				send("GET USER LIST");
				send("GET CHAN LIST");
			}
			else if (args[0].equals("SERVER"))
			{
				args = msg.split(" ");
				serverVersion = args[args.length - 1];
				msg = "<font color=\"#7070c0\">" + msg + "</font>";
			}
		}
		
		return msg;
	}
	
	public void login(String login, String password)
	{
		if (state == LOGGED)
		{
			close();
			connect();
		}
		send("LOGIN " + login + " " + password);
		this.login = login;
	}

	public void register(String login, String password)
	{
		if (state == LOGGED)
		{
			close();
			connect();
		}
		send("REGISTER " + login + " " + password);
		this.login = login;
	}
	
	public void logout()
	{
		send("DISCONNECT");
	}
	
	private void removeUser(String user)
	{
		userListChanged = true;
		userList.remove(user);
	}
	
	private void addUser(String user)
	{
		removeUser(user);
		userList.add(user);
		Collections.sort(userList);
	}

	private void removeChan(String chan)
	{
		chanListChanged = true;
		chanList.remove(chan);
	}
	
	private void addChan(String chan)
	{
		removeChan(chan);
		chanList.add(chan);
		Collections.sort(chanList);
	}
	
	public String unescape(String msg)
	{
		StringBuilder buf = new StringBuilder();
		for(int i = 0 ; i < msg.length() ; ++i)
			if (msg.charAt(i) != '\\')
				buf.append(msg.charAt(i));
			else
			{
				++i;
				buf.append(msg.charAt(i));
			}
		return buf.toString();
	}

	public String escape(String msg)
	{
		StringBuilder buf = new StringBuilder();
		for(int i = 0 ; i < msg.length() ; ++i)
		{
			if (msg.charAt(i) == '\\' || msg.charAt(i) == '\"')
				buf.append('\\');
			buf.append(msg.charAt(i));
		}
		return buf.toString();
	}
	
	public LinkedList<String> getUserList()
	{
		return userList;
	}

	public LinkedList<String> getChanList()
	{
		return chanList;
	}
	
	public String getLogin()
	{
		return login;
	}
	
	public boolean userListHasChanged()
	{
		boolean tmp = userListChanged;
		userListChanged = false;
		return tmp;
	}

	public boolean chanListHasChanged()
	{
		boolean tmp = chanListChanged;
		chanListChanged = false;
		return tmp;
	}
}

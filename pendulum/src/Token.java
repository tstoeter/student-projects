public class Token
{
	public String str;
	public TokenType type;
	public int prio;

	public Token(String s, TokenType t)
	{
		str = s;
		type = t;
		prio = 0;
	}

	public String toString()
	{
		int t = 0;

		if (type != TokenType.INVALID)
			t = 1;
			
		return str + ":" + t;
	}
}


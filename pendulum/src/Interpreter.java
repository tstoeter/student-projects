import java.util.*;

public class Interpreter
{
	LinkedList<Token> tokens;
	Stack<Double> nums;
	Stack<Token> ops;
	int pcount;

	LinkedList<Double> valtab;

	public Interpreter(LinkedList<Token> t, LinkedList<Double> vt)
	{
		tokens = t;

		// null operation hinufügen
		Token nop = new Token("#", TokenType.SYMBOL);
		tokens.addLast(nop);

		// private variablen initialisieren
		nums = new Stack<Double>();
		ops = new Stack<Token>();
		pcount = 0;

		valtab = vt;
	}

	public void setValueTable(LinkedList<Double> vt)
	{
		valtab = vt;
	}

	private int priority(Token op)
	{
		int p;

		// priorität eine operation berechnen, abhängig von der klammertiefe
		if (op.str.equals("#"))	// null operation mit kleinster priorität, kommt am ende des ausdrucks und zwingt zur abbarbeitung des stacks
		{
			p = -1;
		}
		else if (op.str.equals("+") || op.str.equals("-"))
		{
			p = 4*pcount + 0;
		}
		else if (op.str.equals("*") || op.str.equals("/"))
		{
			p = 4*pcount + 1;
		}
		else if (op.str.equals("^"))
		{
			p = 4*pcount + 2;
		}
		else		// sonst funktionen im string
		{
			p = 4*pcount + 3;
		}

		return p;
	}

	private double compute(String op, double x, double y)
	{
		double r = 0;

		if (op.equals("+"))
		{
			r = x + y;
		}
		else if (op.equals("-"))
		{
			// reihenfolge in der vom stack gepoppt wird beachten
			r = y - x;
		}
		else if (op.equals("*"))
		{
			r = x * y;
		}
		else if (op.equals("/"))
		{
			// reihenfolge in der vom stack gepoppt wird beachten
			r = y / x;
		}
		else if (op.equals("^"))
		{
			// reihenfolge in der vom stack gepoppt wird beachten
			r = Math.pow(y, x);
		}
		else if (op.equals("exp"))
		{
			r = Math.exp(x);
		}
		else if (op.equals("ln"))
		{
			// java Math.log ist mathematisches ln
			r = Math.log(x);
		}
		else if (op.equals("lg") || op.equals("log"))
		{
			r = Math.log10(x);
		}
		else if (op.equals("ld"))
		{
			r = Math.log(x)/Math.log(2);
		}
		else if (op.equals("sqrt"))
		{
			r = Math.sqrt(x);
		}
		else if (op.equals("cbrt"))
		{
			r = Math.cbrt(x);
		}
		else if (op.equals("sin"))
		{
			r = Math.sin(x);
		}
		else if (op.equals("cos"))
		{
			r = Math.cos(x);
		}
		else if (op.equals("tan"))
		{
			r = Math.tan(x);
		}
		else if (op.equals("cot"))
		{
			r = 1/Math.tan(x);
		}
		else if (op.equals("asin"))
		{
			r = Math.asin(x);
		}
		else if (op.equals("acos"))
		{
			r = Math.acos(x);
		}
		else if (op.equals("atan"))
		{
			r = Math.atan(x);
		}
		else if (op.equals("sinh"))
		{
			r = Math.sinh(x);
		}
		else if (op.equals("cosh"))
		{
			r = Math.cosh(x);
		}
		else if (op.equals("tanh"))
		{
			r = Math.tanh(x);
		}
		else if (op.equals("abs"))
		{
			r = Math.abs(x);
		}
		else if (op.equals("sign"))
		{
			r = Math.signum(x);
		}
		else if (op.equals("ceil"))
		{
			r = Math.ceil(x);
		}
		else if (op.equals("floor"))
		{
			r = Math.floor(x);
		}
		else if (op.equals("round"))
		{
			r = Math.rint(x);
		}

		return r;
	}

	public double evaluate()
	{
		// iterator über tokenliste
		ListIterator<Token> iter = tokens.listIterator(0);

		// liste durchgehen
		while (iter.hasNext())
		{
			// und jeden einzelnen token betrachten
			Token token = iter.next();

			// und auf entsprechenden stack schieben
			if (token.type == TokenType.NUMBER)
			{
				nums.push(Double.parseDouble(token.str));
			}
			else if (token.type == TokenType.VARIABLE)
			{
				int index = Integer.parseInt(token.str);
				nums.push(valtab.get(index));
			}
			else if (token.type == TokenType.PARANTHESIS)
			{
				if (token.str.equals("("))
					pcount++;
				else
					pcount--;
			}
			else	// oparationen oder funktionen
			{
				token.prio = priority(token);

				// wenn nichts auf dem ops stack, pushen und nächster durchlauf
				if (ops.empty())
				{
					ops.push(token);
					continue;
				}

				// prioritäten vergleichen, wenn aktuelles element kleinere oder gleiche priorität hat
				// alle operationen poppen und ausführen, bis geringere priorität erreicht
				// vergleich nur, wenn was auf dem stack liegt (siehe oben)
				while (token.prio <= ops.peek().prio)
				{
					Token t = ops.pop();
					Double r = 0.0;

					// funktionen sind unäre operationen
					if (t.type == TokenType.FUNCTION)
						r = compute(t.str, nums.pop(), 0);
					else if (t.type == TokenType.SYMBOL)	// binäre operationen
						r = compute(t.str, nums.pop(), nums.pop());

					// ergebnis auf nums stack pushen
					nums.push(r);

					// abbrechen, wenn stack komplett abgearbeitet
					if (ops.empty())
						break;
				}

				// priorität nun höher als operationen auf stack, daher pushen
				ops.push(token);
			}
		}

		return nums.pop().doubleValue();
	}
}


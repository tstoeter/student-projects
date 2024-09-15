import java.awt.*;
import java.awt.event.*;


public class Main extends Frame implements ActionListener, WindowListener
{
	TextField prof;
	TextField x, gam;
	Button apply;

	Processor proc;
	String hstr;
	boolean error = false;

	Graphics buffer;
	Image bitmap;
	Dimension screen;
	Insets insets;
	long oldtime, time;

	double t = 0.0;
	double h = 0.01;
	double l = 2.0;
	double gamma;
	double[] y = {0,0};
	double[] p = {0,0,0};
	int i;

	Pendulum pendulum;

	public Main()
	{
		// init pendulum data structures
		t = y[0] = y[1] = gamma = p[0] = p[1] = p[2] = 0;
		pendulum = new Pendulum(l, gamma, y, t, h);
		
		// init arithmetic processor
		proc = new Processor();
		proc.setVariable("t", t);

		// init gui		
		this.setSize(400,400);
		this.setVisible(true);
		this.setLayout(null);
		this.setTitle("Vertically Driven Pendulum");
		
		// init double buffer
		screen = this.getSize();
		insets = getInsets();
		screen.width -= (insets.left + insets.right);
		screen.height -= (insets.top + insets.bottom);
		bitmap = createImage(screen.width, screen.height);
		buffer = bitmap.getGraphics();
		oldtime = System.currentTimeMillis();
		
		apply = new Button("run");
		prof = new TextField("0.5*sin(25*t)", 80);
		x = new TextField("0.7", 80);
		gam = new TextField("0.0", 80);
		hstr = new String("0");

		prof.setBounds(insets.left+60,insets.top+20,250,25);
		x.setBounds(insets.left+60,insets.top+60,50,25);
		gam.setBounds(insets.left+190,insets.top+60,50,25);
		apply.setBounds(insets.left+260,insets.top+60,50,25);

		add(prof);
		add(x);
		add(gam);
		add(apply);

		apply.addActionListener(this);
		addWindowListener(this);
	}
	
	public void paint(Graphics g)
	{
		double ppm = 50;
		buffer.clearRect(0, 0, screen.width, screen.height);

		buffer.drawString("h =", 31, 36);
		buffer.drawString("x0 =", 25, 76);
		buffer.drawString("gamma =", 123, 76);

		if (error)
			buffer.drawString("input failure", 150, 10+80);

		double p1 = p[(3+i-1)%3];

		buffer.drawLine(screen.width/2-10, screen.height/2-(int)(p1*ppm), screen.width/2+10, screen.height/2-(int)(p1*ppm));
		buffer.drawLine(screen.width/2, screen.height/2-(int)(p1*ppm), screen.width/2+(int)(l*Math.sin(y[1])*ppm), screen.height/2-(int)(p1*ppm)+(int)(l*Math.cos(y[1])*ppm));
		buffer.fillOval(screen.width/2+(int)(l*Math.sin(y[1])*ppm)-10, screen.height/2-(int)(p1*ppm)+(int)(l*Math.cos(y[1])*ppm)-10, 20, 20);

		g.drawImage(bitmap, insets.left, insets.top, this);

		repaint();
	}

	public void update(Graphics g)
	{
		time = System.currentTimeMillis();

		if (time-oldtime >= (long)(1000*h))
		{
			// do one integration step and increment time
			pendulum.SingleStep(y);
			t += h;

			// compute profile for future time step
			proc.setVariable("t", t+h);
			proc.process("p = " + hstr);
			p[(3+i-1)%3] = proc.getVariable("p");
			i++;

			// and update values for second order derivative
			pendulum.update(p, i);

			oldtime = time;
		}
		
		paint(g);
	}

	public void actionPerformed(ActionEvent evt)
	{
		error = false;

		try
		{
			hstr = prof.getText();

			t = 0.0;

			y[0] = 0.0;
			y[1] = Double.parseDouble(x.getText());

			gamma = Double.parseDouble(gam.getText());

			proc.setVariable("t", t-h);
			proc.process("p = " + hstr);
			p[0] = proc.getVariable("p");

			proc.setVariable("t", t+h);
			proc.process("p = " + hstr);
			p[2] = proc.getVariable("p");

			proc.setVariable("t", t);
			proc.process("p = " + hstr);
			p[1] = proc.getVariable("p");

			i = 1;
		}
		catch (RuntimeException e)
		{
			t = y[0] = y[1] = gamma = p[0] = p[1] = p[2] = 0;
			hstr = new String("0");

			error = true;
		}

		pendulum = new Pendulum(l, gamma, y, t, h);
		pendulum.update(p, i);

		repaint();
	}

	public static void main(String[] args)
	{
		new Main();
	}

	public void windowActivated(WindowEvent e) {}
   public void windowClosed(WindowEvent e) {}
   public void windowClosing(WindowEvent e)
  	{
  		dispose();
   }    
   public void windowDeactivated(WindowEvent e) {}    
   public void windowDeiconified(WindowEvent e) {}    
   public void windowIconified(WindowEvent e) {}    
   public void windowOpened(WindowEvent arg0) {}
}

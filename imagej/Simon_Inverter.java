import ij.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;
import ij.plugin.filter.*;

public class Simon_Inverter implements PlugInFilter {
	ImagePlus imp;

	public int setup(String arg, ImagePlus imp) {
		this.imp = imp;
		return DOES_8G;
	}

	public void run(ImageProcessor ip) {
		int w = ip.getWidth();
		int h = ip.getHeight();

		for (int v = 0; v < h; v++)
		{
			for (int u = 0; u < w; u++)
			{
				int p = ip.get(u, v);
				p = 255 - p;
				ip.set(u, v, p);
			}
		}
	}

}

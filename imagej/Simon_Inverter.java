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

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				int p = ip.getPixel(x, y);
				p = 255 - p;
				ip.putPixel(x, y, p);
			}
		}
		
		ImageProcessor ip2 = new ByteProcessor(10, 10);
		ip2.putPixel(5, 5, 0);
		ImagePlus ipp2 = new ImagePlus("Yo!", ip2);
		ipp2.show();
	}

}

import ij.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;
import ij.plugin.filter.*;

public class Simon_Inverter implements PlugInFilter {
	ImagePlus imp;

	public int setup(String arg, ImagePlus imp) {
		this.imp = imp;
		return DOES_8G | DOES_RGB;
	}

	public void run(ImageProcessor ip) {
		int w = ip.getWidth();
		int h = ip.getHeight();

		if (ip.getNChannels() == 1) {
			invertGrayscale(w, h, ip);
		} else if (ip.getNChannels() == 3) {
			invertRGB8bpp(w, h, ip);
		} else {
			throw new RuntimeException("Only supports 8-bit Grayscale or 8bpp RGB.");
		}
	}

	private void invertGrayscale(int w, int h, ImageProcessor ip) {
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

	private void invertRGB8bpp(int w, int h, ImageProcessor ip) {
		for (int v = 0; v < h; v++)
		{
			for (int u = 0; u < w; u++)
			{
				int p = ip.get(u, v);

				int r = 255 - ((p & 0xFF0000) >> 16);
				int g = 255 - ((p & 0x00FF00) >> 8);
				int b = 255 - ((p & 0x0000FF) >> 0);

				ip.set(u, v, (r << 16) | (g << 8) | b);
			}
		}
	}
}

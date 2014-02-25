import ij.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;
import ij.plugin.filter.*;

// Increase contrast 50%.
public class SO_IncreaseContrast50 implements PlugInFilter {
	final float CONTRAST_INCREASE = 1.5f;

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
				int p = (int) ((float) ip.get(u, v) * CONTRAST_INCREASE + 0.5f);
				if (p > 255)
					p = 255;
				ip.set(u, v, p);
			}
		}
	}

}

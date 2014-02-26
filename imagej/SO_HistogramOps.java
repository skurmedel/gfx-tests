import ij.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;
import ij.plugin.filter.*;

import java.util.Arrays;
import java.util.Collections;

/*
	This outputs a new B&W histogram for a 16-bit or 8-bit
	gray image.

	Note that this will in some situations not match the 
	displayed histograms in Photoshop or ImageJ. PS and IJ
	seem to use an average or saturated edge max count when 
	calculating the average height of each bar in the histo-
	gram. This means that they actually don't display the 
	whole histogram in some situations where the count for
	one or two colors are extremely dominant.

	This is often more useful, but alas, I can't be arsed
	to implement it here, and I want my histoimages to be 
	true to the relative values.
*/

public class SO_HistogramOps implements PlugInFilter {
	final int HI_WIDTH = 256;
	final int HI_HEIGHT= 128;

	ImagePlus imp;

	public int setup(String arg, ImagePlus imp) {
		this.imp = imp;
		return DOES_8G | DOES_16 | NO_CHANGES;
	}

	public void run(ImageProcessor ip) {

		final int bins = HI_WIDTH;

		int[] histo = computeHistogram(ip, bins);

		int maxPixels = ip.getWidth() * ip.getHeight();
		int maxVal = max(histo);

		ImageProcessor histoip = new ByteProcessor(bins, HI_HEIGHT);
		histoip.setValue(255); histoip.fill();

		for (int v = 127; v > 0; v--) {
			for (int u = 0; u < bins; u++) {
				double relheight = (HI_HEIGHT - v) / (double) HI_HEIGHT;
				double relval = histo[u] / (double) maxVal;
				if (relheight < relval)	{
					histoip.set(u, v, 0);
				}
			}
		}

		ImagePlus nimg = new ImagePlus("Histogram for " + imp.getTitle(), histoip);
		nimg.show();
	}

	private int[] computeHistogram(ImageProcessor ip, int bins) {
		int maxValues = 256;
		if (ip.getBitDepth() == 16)
		{
			maxValues = 65536;
		}

		int w = ip.getWidth();
		int h = ip.getHeight();

		double bsize = bins / (double) maxValues;

		int[] histo = new int[bins];

		for (int v = 0; v < h; v++)
		{
			for (int u = 0; u < w; u++)
			{
				int p = ip.get(u, v);
				
				int i = (int) (p * bsize);
				histo[i] = histo[i] + 1;
			}
		}

		return histo;
	}

	private int max(int[] vals) {
		int n = Integer.MIN_VALUE;
		for (int v : vals) {
			if (v > n)
				n = v;
		}
		return n;
	}

}

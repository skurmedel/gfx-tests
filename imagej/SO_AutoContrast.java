import ij.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;
import ij.plugin.filter.*;
import java.lang.*;

import java.util.Arrays;
import java.util.Collections;

/*
	This operator tries to maximise the contrast range
	of an image by finding the darkest and brightest
	regions and extending the contrast to the full range
	offered by the bitdepth.
*/

public class SO_AutoContrast implements PlugInFilter {
	/*
		0.5% is what Photoshop uses to saturate the edges
		of the histogram before picking the brightest
		and darkest values.

		It basically makes fringe bright and dark spots less
		relevant for the end result.
	*/
	final float QUANTILE_PHOTOSHOP = 0.005f;

	ImagePlus imp;

	public int setup(String arg, ImagePlus imp) {
		this.imp = imp;
		return DOES_8G | DOES_16;
	}

	public void run(ImageProcessor ip) {
		int histo[] = ip.getHistogram();

		int cHisto[] = makeCumulative(histo);

		int totalPixels = ip.getWidth() * ip.getHeight();

		int lowCount =  (int) (QUANTILE_PHOTOSHOP * totalPixels);
		int highCount = (int) ((1.0f - QUANTILE_PHOTOSHOP) * totalPixels);

		int low = 0;
		int high = ip.getBitDepth() == 16? 65535 : 255;
		
		for (int i = 0; i < cHisto.length; i++)
		{
			int c = cHisto[i];
			if (c >= lowCount) {
				low = i;
				break;
			}
			
		}

		for (int i = cHisto.length - 1; i > -1; i--)
		{
			int c = cHisto[i];
			if (c <= highCount) {
				high = i;
				break;
			}			
		}		

		if (low == high)
			return;

		IJ.showMessage("My_Plugin", "Low: " + Integer.toString(low) + " High: " + Integer.toString(high));

		modifyContrast(ip, low, high);
	}

	private void modifyContrast(ImageProcessor ip, int low, int high) {
		
		if (ip.getBitDepth() == 16)
		{
			throw new RuntimeException("Not implemented 16-bit support.");
		}

		float min = 0.0f;
		float max = ip.getBitDepth() == 16? 65535.0f : 255.0f;

		int w = ip.getWidth();
		int h = ip.getHeight();

		for (int v = 0; v < h; v++)
		{
			for (int u = 0; u < w; u++)
			{
				int p = ip.get(u, v);

				if (p <= low)
					p = (int) min;
				else if (p >= high) 
					p = (int) max;
				else
					p = (int) ( min + (p - low) * ((max - min) / (high - low)) );
				ip.set(u, v, p);
			}
		}		
	}
	/*
	private short lerp16(short low, short high, float a) {

	}
	*/

	private int[] makeCumulative(int[] histo) {
		int n = 0; 
		for (int i = 0; i < histo.length; i++) {
			n += histo[i];
			histo[i] = n;
		}
		return histo;
	}
}

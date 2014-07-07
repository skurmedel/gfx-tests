import ij.*;
import ij.io.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;
import java.io.*;
import java.awt.event.*;
import ij.plugin.filter.*;
import java.lang.*;

import java.util.Arrays;
import java.util.Collections;

/*
	A Z-blur linear filter. Z-blur is a common thing in 
	VFX and games, where you simulate Depth of Field (bokeh)
	through a selective blur. Areas determined to be in 
	focus are less blurred than areas not in focus.
	The amount depends on their scene depth.

	The scene depth is acquired through a separate image,
	of the same dimensions, which is grayscale and encodes
	the depth for each RGB value.
*/

public class SO_ZBlur implements ExtendedPlugInFilter, DialogListener {
	private static final int FLAGS = DOES_RGB;
	ImagePlus imp;

	double focalPoint = 0.0;
	double blurSize = 0.0;
	String zPassPath = "";
	ImagePlus zPass;

	public void setNPasses(int nPasses) {

	}

	public int showDialog(ImagePlus img, String cmd, PlugInFilterRunner pfr) {
		
		GenericDialog gd = new GenericDialog("ZBlur");
		gd.addSlider("Focal point: ", -0.5, 0.5, 0.0);
		gd.addSlider("Blur size: ", 0.0, 3.0, 1.25);

		Button fileSelection = new Button("Select Z-pass...");
		fileSelection.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
            	OpenDialog dialog = new OpenDialog("Select Z-pass", "", "zPassPath.tiff");
            	zPassPath = dialog.getPath();
            }
        });
		gd.addPreviewCheckbox(pfr);
		gd.addDialogListener(this);
		gd.add(fileSelection);
		gd.showDialog();
		if (gd.wasCanceled()) 
			return NO_CHANGES;

		return FLAGS;
	}

	public int setup(String arg, ImagePlus imp) {
		this.imp = imp;
		return FLAGS;
	}

	public void run(ImageProcessor ip) {
		// Read the Z-pass.
		if (!ReadZPass()) {
			IJ.showMessage("The selected z-pass image was not a valid image.");
			return;
		}

		int zPassBits = this.zPass.getProcessor().getBitDepth();
		//IJ.showMessage(Integer.toString(zPassBits));
		FloatProcessor zPassProcessor = (FloatProcessor) this.zPass.getProcessor().convertToFloat();
		double zPassScale = 1.0;
		if (zPassBits == 16) {
			zPassScale = 1.0 / 65535.0;
		} else if (zPassBits == 8) {
			zPassScale = 1.0 / 255.0;
		}

		DoZBlur((ColorProcessor) ip, zPassProcessor, zPassScale);

		// Close the Z-pass.
		if (this.zPass != null)
			this.zPass.close();

		return;
	}

	private double gauss_pdf(double x, double mju, double sigma) {
		double a = 1 / (sigma * Math.sqrt(2.0 * Math.PI));
		double div_squared = (x - mju) / sigma;
			  div_squared *= div_squared;
		return a * Math.exp(-0.5 * div_squared);
	}

	private double[] get1DKernel(int width) {
		assert width > 1;

		double[] kernel = new double[width];

		for (int i = -(width / 2); i < width / 2 + 1; i++) {
			kernel[i + width/2] = gauss_pdf(i, 0, 5);
		}

		return kernel;
	}

	private int clamp8bit(int v) {
		if (v < 0)
			return 0;
		if (v > 255)
			return 255;
		return v;
	}

	private int packToInt(double r, double g, double b) {
		int r2 = ( clamp8bit((int) (r * 255)) ) << 16 ;
		int g2 = ( clamp8bit((int) (g * 255)) ) <<  8 ;
		int b2 = ( clamp8bit((int) (b * 255)) )       ;
		return r2 | g2 | b2;
	}

	private void DoZBlur(ColorProcessor ip, FloatProcessor zpassp, double zPassScale) {
		
		// IJ.showMessage(ip.getClass().toString());

		final int kWidth = 41;
		double[] kernel = get1DKernel(kWidth);
		final int halfKWidth = kWidth / 2;

		for (int y = 0; y < ip.getHeight(); y++)	{
			for (int x = 0; x < ip.getWidth(); x++)	{
				double r = 0;
				double g = 0;
				double b = 0;

				double scale = zPassScale * zpassp.getf(x, y) * blurSize;

				for (int i = -halfKWidth; i < halfKWidth + 1; i++) {
					double offset = y + (scale * i);
					
					if (offset < 0)
						offset = 0;
					if (offset > ip.getHeight() - 1.00)
						offset = ip.getHeight() - 1.00;

					int p = ip.getInterpolatedRGBPixel((double) x, offset);

					r += kernel[i + halfKWidth] * (((p >> 16) & 0xFF) / 255.0);
					g += kernel[i + halfKWidth] * (((p >> 8)  & 0xFF) / 255.0);
					b += kernel[i + halfKWidth] * (((p >> 0)  & 0xFF) / 255.0);
				}
				
				ip.set(x, y, packToInt(r, g, b));
			}
		}

		for (int y = 0; y < ip.getHeight(); y++)	{
			for (int x = 0; x < ip.getWidth(); x++)	{
				double r = 0;
				double g = 0;
				double b = 0;

				double scale = zPassScale * zpassp.getf(x, y) * blurSize;

				for (int i = -halfKWidth; i < halfKWidth + 1; i++) {
					double offset = x + (scale * i);
					if (offset < 0)
						offset = 0;
					if (offset > ip.getWidth() - 1.00)
						offset = ip.getWidth() - 1.00;
					int p = ip.getInterpolatedRGBPixel(offset, (double) y);
					r += kernel[i + halfKWidth] * (((p >> 16) & 0xFF) / 255.0);
					g += kernel[i + halfKWidth] * (((p >> 8)  & 0xFF) / 255.0);
					b += kernel[i + halfKWidth] * (((p >> 0)  & 0xFF) / 255.0);
				}
				
				
				ip.set(x, y, packToInt(r, g, b));
			}
		}

		
	}

	public boolean dialogItemChanged(GenericDialog dg, java.awt.AWTEvent e)	{
		// Prevent running the plugin if the path to the z-pass was invalid.
		if (!new File(zPassPath).exists()) {
			IJ.showMessage("The path \"" + zPassPath + "\" does not exist.");
			return false;
		}

		focalPoint = dg.getNextNumber();
		blurSize = dg.getNextNumber();

		return true;
	}

	private boolean ReadZPass() {
		Opener o = new Opener();
		this.zPass = o.openImage(zPassPath);
		if (this.zPass == null)
			return false;

		//IJ.showMessage(String.format("Bitdepth: %d. Dims: %d:%d.", this.zPass.getBitDepth(), this.zPass.getWidth(), this.zPass.getHeight()));

		return true;
	}
}

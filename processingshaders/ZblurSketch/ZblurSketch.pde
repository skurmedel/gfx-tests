import processing.core.*;
import processing.opengl.*;

public class ZblurSketch extends PApplet {

	PImage img;
        PImage zImg;
	PShader zblurShader;
        PShape can;

	public void setup() {
		float aspect = 16.0f/9.0f;
		size(1280,(int) (1280.0/aspect), P2D);
		img = loadImage("bunnies.JPG");
                zImg = loadImage("bunnies_z8bit.png");
		zblurShader = loadShader("zblur_es.glsl");
                if (zblurShader == null) {
                  print("Shader load failed.");
                }
                rectMode(CENTER);
                zblurShader.set("zTex", zImg);
	}

	public void draw() {
		background(0);
                zblurShader.set("ratio", 0.4);
                zblurShader.set("focusPoint", mouseX / (float)width, mouseY / (float)height);
		shader(zblurShader);
	        image(img, 0, 0, width, height);
	}
}

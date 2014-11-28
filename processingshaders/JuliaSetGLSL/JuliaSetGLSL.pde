import processing.core.*;
import processing.opengl.*;

public class JuliaSetGLSL extends PApplet {

  PGraphics rb;
  PShader fractalShader;
  PImage gradient1; 
  
  final int ssample = 2;

  final float zoomMin = 2.5;
  final float zoomMax = 0.001;

  float zoom = 499;
  int zoomPressed = 0;

  public void setup() {
    float aspect = 16.0f/9.0f;
    size(1650,(int) (1650.0/aspect), P2D);
    
    rb = createGraphics(width * ssample, height * ssample, P3D);
    
    gradient1 = loadImage("gradient1.png");
    fractalShader = rb.loadShader("julia_es.glsl");
    fractalShader.set("aspect", aspect);
    fractalShader.set("off", 1.0f / (float)width * ssample, 1.0f / (float)height * ssample);
    fractalShader.set("gradient", gradient1);
    if (fractalShader == null) {
      print("Shader load failed.");
    }
  }

  public void draw() {
    float zoomFactor = 0.00001*(zoom*zoom);    
    if (zoomFactor < zoomMax)
    {
      zoomFactor = zoomMax;
      zoomPressed = 0;
      zoom += 1;
    }
    else if (zoomFactor > zoomMin)
    {
      zoomFactor = zoomMin;
      zoomPressed = 0;
      zoom -= 1;
    }
    zoom += zoomPressed;  
        
    rb.beginDraw();
    rb.background(0);
    fractalShader.set("mousePoint", mouseX / (float)width, mouseY / (float)height);
    fractalShader.set("zoom", zoomFactor);
    rb.shader(fractalShader);
    rb.rect(0, 0, width * ssample, height * ssample);
    rb.endDraw();
    image(rb, 0, 0, width, height);
  }

  void mousePressed()
  {    
    if (mouseButton == LEFT)
    {
      zoomPressed = 1;
    }
    else
    {
      zoomPressed = -1;
    }
  }
  void mouseReleased()
  {
    zoomPressed = 0;
  }
}

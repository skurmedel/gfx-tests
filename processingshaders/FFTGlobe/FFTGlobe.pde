import ddf.minim.*;
import ddf.minim.analysis.*;

Minim minim;
AudioInput audioInput;
FFT fft;

PShader sphereShader;
PShader bloomShader;
PGraphics buffer3d;

PFont font;

float beatDecay;

void setup()
{
  size(1024, 1024, P3D);
  frameRate(30);
  
  font = createFont("Arial", 32.0);
  textFont(font);
  
  buffer3d = createGraphics(width, height, P3D);
  buffer3d.sphereDetail(80);
  buffer3d.perspective(PI/2, float(width) / float(height), 0, 900.0);
  
  sphereShader = loadShader("fft_sphere_es.glsl", "fft_sphere_es_vs.glsl");
  sphereShader.set("beat", 0.0);
  bloomShader = loadShader("bloom_es.glsl");
  
  minim = new Minim(this);
  audioInput = minim.getLineIn(Minim.MONO, 2048);
  println("Audio Input uses Buffer Size: " + Integer.toString(audioInput.bufferSize()));
  
  
  fft = new FFT(audioInput.bufferSize(), audioInput.sampleRate());
  fft.window(FFT.HAMMING);
  fft.noAverages();
  
  ortho();
  rectMode(CENTER);
}

float accentuateBand(float avgAmplitude)
{
  return (float) Math.pow(Math.log(avgAmplitude + 1.0), 1.75);
}

void draw3d()
{
  buffer3d.beginDraw();
    fft.forward(audioInput.left);
    float bassDetect = accentuateBand(fft.calcAvg(20.0, 100.0));
    float midDetect = accentuateBand(fft.calcAvg(500.0, 1500.0));
    float highDetect = accentuateBand(fft.calcAvg(3000.0, 10000.0));
     
    buffer3d.colorMode(RGB, 255);  
    buffer3d.background(0);
    
    buffer3d.hint(DISABLE_DEPTH_TEST);
    
    sphereShader.set("beat", bassDetect);
    buffer3d.pushMatrix();
      buffer3d.translate(512, 512, 715);
      buffer3d.scale(2 + bassDetect * 0.005);
      buffer3d.noStroke();
      buffer3d.shader(sphereShader);
      buffer3d.sphere(30.0);
      buffer3d.scale(1.01);
      buffer3d.shader(sphereShader);
      buffer3d.sphere(30.0);
    buffer3d.popMatrix();
  buffer3d.endDraw(); 
}

void draw()
{    
  draw3d();
  background(0);
  shader(bloomShader);
  PImage img = buffer3d.get();
  beginShape(QUADS);
    textureMode(NORMAL);
    texture(buffer3d);
    //fill(color(255, 0, 0));
    vertex(0, 0, 0, 0);
    vertex(width, 0, 1, 0);
    vertex(width, height, 1, 1);
    vertex(0, height, 0, 1);
  endShape();
  
  resetShader();
  fill(255,255,255);
  textSize(32);
  text("FPS: " + Float.toString(frameRate), 30, 30);
}

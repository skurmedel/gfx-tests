import ddf.minim.*;
import ddf.minim.analysis.*;

final int W_WIDTH   = 1024;
final int W_HEIGHT  = int(float(W_WIDTH) * (9.0/16.0));
final int DESIRED_FPS = 50;
final int INPUT_BUFSIZE = 1024;

float FOV = PI / 4;
float CAMERA_Z = (float(W_HEIGHT) * 0.5) / tan(FOV * 0.5);

/* -------------------------------------------
  GLOBALS
------------------------------------------- */
Minim minim;
AudioInput audioInput;
FFT fft;

PShader sphereShader;
PShader bloomShader;
PGraphics buffer3d;

PFont font;

float beatDecay;  

/* -------------------------------------------
  MAIN SCENE RENDER
------------------------------------------- */

void setupScene()
{
  buffer3d = createGraphics(width, height, P3D);
  buffer3d.sphereDetail(80);
  
  sphereShader = loadShader("fft_sphere_es.glsl", "fft_sphere_es_vs.glsl");
  sphereShader.set("beat", 0.0);
}

String prettyFormatHz(float hz)
{
  if (hz > 999.0f)
  {
    return String.format("%.1f kHz", hz * 0.001);
  }
  return String.format("%.1f Hz", hz);
}

float maxAmplitude = -1.0;
void drawSpectrum(PGraphics g)
{
  fft.logAverages(500, 8);
  
  fft.forward(audioInput.left);
  
  float spectrumWidth = 80.0f;
  float step = spectrumWidth / fft.avgSize();
  float spectrumHeight = 40.0f;
  
  float baseX = spectrumWidth * -0.5;
  float baseY = 20.0;
  
  float amplitudes[] = new float[fft.avgSize()];
  float x[] = new float[fft.avgSize()];
  float y[] = new float[fft.avgSize()];

  for (int i = 0; i < fft.avgSize(); i++)
  {
    float a = fft.getAvg(i);
    if (a > maxAmplitude)
      maxAmplitude = a;
  }
  
  for (int i = 0; i < fft.avgSize(); i++)
  {
    amplitudes[i] = fft.getAvg(i) / maxAmplitude;
    if (amplitudes[i] > 1.0f)
      println("Omg, ", amplitudes[i]);
    x[i] = baseX + i * step;
    y[i] = baseY 
         - spectrumHeight 
         * log(1.0 + amplitudes[i] * 1.71828182f);
  }
  
  g.noFill();
  
  g.stroke(204, 103, 41, 15);
  g.beginShape(LINES);
    for (int i = 0; i < fft.avgSize(); i++)
    {
      g.vertex( baseX, y[i]);
      g.vertex(-baseX, y[i]);
      g.vertex(  x[i], y[i], 0.0);
      g.vertex(  x[i], baseY - spectrumHeight + 1, 0.0);
    }
  g.endShape();
  
  g.noStroke();
  g.fill(255, 255, 255,  80);
  g.textFont(font);
  g.textSize(0.8);
  g.textAlign(CENTER);
  for (int i = 1; i < fft.avgSize() - 1; i+=3)
  {
    g.ellipse(x[i], y[i], 0.5, 0.5);
    g.text(String.format("%.1f", amplitudes[i]), x[i], y[i] - 2);
    g.text(prettyFormatHz(fft.indexToFreq(i)), x[i], baseY - spectrumHeight);
  }
  
  g.noFill();
  g.stroke(255, 255, 255);
  g.beginShape();
    g.curveVertex(x[0] - 0.1, y[0]);
    g.curveVertex(x[0], y[0]);
    for (int i = 1; i < fft.avgSize() - 1; i++)
    {
      g.curveVertex(x[i], y[i]);
    }
    g.vertex(x[fft.avgSize() - 1], y[fft.avgSize() - 1]);
    g.vertex(x[fft.avgSize() - 1] + 0.1, y[fft.avgSize() - 1]);
  g.endShape();
}

void renderScene()
{
  buffer3d.beginDraw();
    /* Processing acts iffy with the perspective if you set it
       up in setup for me, so I do it here instead. */
    buffer3d.resetMatrix();
    buffer3d.perspective(FOV, float(width) / float(height), CAMERA_Z * 0.01, CAMERA_Z * 100);
    buffer3d.translate(0, +0, -55);
    

    buffer3d.colorMode(RGB, 255);  
    buffer3d.background(0);
    
    buffer3d.hint(DISABLE_DEPTH_TEST);
    buffer3d.pushMatrix();
      buffer3d.noStroke();
      //buffer3d.shader(sphereShader);
      //buffer3d.sphere(5);
      buffer3d.fill(255,255,255);
      drawSpectrum(buffer3d);
    buffer3d.popMatrix();
  buffer3d.endDraw(); 
}

/* -------------------------------------------
  SETUP AND DRAW
------------------------------------------- */
void setup()
{
  size(W_WIDTH, W_HEIGHT, P3D);
  frameRate(DESIRED_FPS);
  
  font = loadFont("sansserif.vlw");
  textFont(font);
  
  ortho();
  rectMode(CENTER);
  
  setupScene();

  bloomShader = loadShader("bloom_es.glsl");
  
  minim = new Minim(this);
  audioInput = minim.getLineIn(Minim.MONO, INPUT_BUFSIZE);
  println("Audio Input uses Buffer Size: " + Integer.toString(audioInput.bufferSize()));
  
  fft = new FFT(audioInput.bufferSize(), audioInput.sampleRate());
  fft.window(FFT.HAMMING);
  fft.noAverages();
}

float accentuateBand(float avgAmplitude)
{
  return (float) Math.pow(Math.log(avgAmplitude + 1.0), 1.75);
}

void draw()
{    
  renderScene();
  background(0);
  bloomShader.set("mixAmount", mouseX / float(width));
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



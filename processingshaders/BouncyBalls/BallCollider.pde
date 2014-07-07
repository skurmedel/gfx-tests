import processing.core.*;

class BallCollider
{
  private float radius;
  private PVector pos;
  // Mass of the ball in kilograms.
  private float massKg;
  // Velocity in meters per second (m/s)
  private PVector velMs;
  
  public BallCollider(float radius, PVector pos, float kg, PVector initialVel)
  {
    this.pos = pos;
    this.radius = radius
    this.massKg = kg;
    this.velMs = initialVel;
  }
    
  public void step(float deltaT)
  {
    
  }
  
  public void applyForce(PVector N)
  {
    PVector vel = N.mult(1.0 / massKg).set(a.x * a.x, a.y * a.y, a.z * a.z).mult(0.5);
    velMs += vel;
  }
}

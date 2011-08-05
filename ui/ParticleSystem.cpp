/* @file ParticleSystem.cpp
 */

#include "ParticleSystem.h"

#include "DatatypeDef.h"

#include <IStorm3D.h>

#include <IStorm3D_Particle.h>

namespace ui {

float ParticleSystem::random() {
	return rand()/(float)RAND_MAX;
}

ParticleSystem::ParticleSystem(IStorm3D_ParticleSystem *particleSystem,
															 ParticleSystemType type, 
															 Vector vector1, 
															 Vector vector2, 
															 float lifeTime) : type_(type), vector1_(vector1),
															                   vector2_(vector2), lifeTime_(lifeTime),
																								 pointParticles_(0), lineParticles_(0),
																								 particleNo_(0), velocities_(0), time_(0),
																								 particleSystem_(particleSystem)
																								 
{
	int i;



	// Ah, yes this switch clause is kinda kludgy, but it'll have to do for now 
  float sizeFactor = 1.0f;
  switch(type_) {

		case EXPLOSION:
		case EXPLOSION2:
		case EXPLOSION3:
      if (type_ == EXPLOSION2) sizeFactor = 2.0f;
      if (type_ == EXPLOSION3) sizeFactor = 4.0f;
			particleNo_=100;
			pointParticles_=new Storm3D_PointParticle[particleNo_];
			velocities_=new Vector[particleNo_];
			mass_=1 * sizeFactor;
			heat_=5 * sizeFactor;
			for(i=0;i<particleNo_;i++) {

				pointParticles_[i].center.alpha=0.7f;
				pointParticles_[i].center.color.r=0.7f;
				pointParticles_[i].center.color.g=0.7f;
				pointParticles_[i].center.color.b=0.7f;

				pointParticles_[i].center.position.x=vector1_.x+(random()*2.0f-1.0f) * sizeFactor;
				pointParticles_[i].center.position.y=vector1_.y+(random()*2.0f-1.0f) * sizeFactor;
				pointParticles_[i].center.position.z=vector1_.z+(random()*2.0f-1.0f) * sizeFactor;
				
				pointParticles_[i].center.size=2.0f * sizeFactor;
				
				pointParticles_[i].alive=true;
				float a;
				if(random()<0.01f) {
					a=0.5f * sizeFactor; 
					pointParticles_[i].center.size=0.8f * sizeFactor;
				} else { 
					a=0.15f * sizeFactor;
				}
				velocities_[i].x=(random()-0.5f)*a;
				velocities_[i].y=(random()-0.5f)*a;
				velocities_[i].z=(random()-0.5f)*a;

			}
			break;

		case SPARK:
		case SPARK2:
      if (type_ == SPARK)
			  particleNo_=5;
      else
			  particleNo_=100;
			pointParticles_=new Storm3D_PointParticle[particleNo_];
			velocities_=new Vector[particleNo_];
			mass_=1;
			heat_=20;
			for(i=0;i<particleNo_;i++) {

				pointParticles_[i].center.alpha=1;
				pointParticles_[i].center.color.r=1;
				pointParticles_[i].center.color.g=1;
				pointParticles_[i].center.color.b=1;

				pointParticles_[i].center.position.x=vector1_.x+random()-0.5f;
				pointParticles_[i].center.position.y=vector1_.y+random()+1;
				pointParticles_[i].center.position.z=vector1_.z+random()-0.5f;
				
				pointParticles_[i].center.size=0.2f;
				
				pointParticles_[i].alive=true;
				float a=0.5f;
				velocities_[i].x=(random()-0.5f)*a;
				velocities_[i].y=(random()-0.5f)*a;
				velocities_[i].z=(random()-0.5f)*a;

				velocities_[i]-=vector2_*10000;

			}

			break;

 		case FLAME:
 		case ROCKETTAIL:
      if (type_ == ROCKETTAIL)
			  particleNo_=10;
      else
			  particleNo_=75;
			pointParticles_=new Storm3D_PointParticle[particleNo_];
			velocities_=new Vector[particleNo_];
			mass_=1;
			heat_=1.005f;
			for(i=0;i<particleNo_;i++) {

				pointParticles_[i].center.alpha=0.7f;
				pointParticles_[i].center.color.r=0.1f;
				pointParticles_[i].center.color.g=0.1f;
				pointParticles_[i].center.color.b=0.4f + (random())/5.0f;

				pointParticles_[i].center.position.x=vector1_.x+random()*0.5f-0.25f
				  + vector2_.x * (5+i*1.5f) / 3.0f;
				pointParticles_[i].center.position.y=vector1_.y+random()*0.5f-0.25f
				  + vector2_.y * (5+i*1.5f) / 3.0f;
				pointParticles_[i].center.position.z=vector1_.z+random()*0.5f-0.25f
				  + vector2_.z * (5+i*1.5f) / 3.0f;

				pointParticles_[i].center.size=0.5f;

				pointParticles_[i].alive=true;
				float a;
				if(random()<0.01f) {
					a=0.5f; 
					pointParticles_[i].center.size=0.3f;
				} else { 
					a=0.10f;
				}
				velocities_[i].x = vector2_.x + (random()-0.5f)*a;
				velocities_[i].y = vector2_.y + (random()-0.5f)*a;
				velocities_[i].z = vector2_.z + (random()-0.5f)*a;

			}
			break;

		case DUST:
		case SMOKE:
		case RISINGSMOKE:
      if (type_ == RISINGSMOKE)
			  particleNo_=100;
      else
			  particleNo_=10;
			pointParticles_=new Storm3D_PointParticle[particleNo_];
			velocities_=new Vector[particleNo_];
      //if (type_ == RISINGSMOKE)
      //{
			//  mass_=1;
			//  heat_=5.4f;
      //} else {
			  mass_=0.001f;
			  heat_=0.001f;
      //}
			for(i=0;i<particleNo_;i++) {

        /*
				pointParticles_[i].center.alpha=0.25f;
				pointParticles_[i].center.color.r=0.25f;
				pointParticles_[i].center.color.g=0.25f;
				pointParticles_[i].center.color.b=0.25f;
        */
				pointParticles_[i].center.alpha=1.0f;
				pointParticles_[i].center.color.r=1.0f;
				pointParticles_[i].center.color.g=1.0f;
				pointParticles_[i].center.color.b=1.0f;

				pointParticles_[i].center.position.x=random()*2.0f-1.0f;
				pointParticles_[i].center.position.y=random()*2.0f-1.0f;
				pointParticles_[i].center.position.z=random()*2.0f-1.0f;
				
				pointParticles_[i].center.position.Normalize();
				pointParticles_[i].center.position*=(random()+0.5f)*0.5f;
				pointParticles_[i].center.position+=vector1_;

        if (type_ == RISINGSMOKE)
        {
          pointParticles_[i].center.position.y -= i / 5;
        }
        if (type_ == DUST)
				  pointParticles_[i].center.size=0.5f;
        else
				  pointParticles_[i].center.size=2.0f;
				
				pointParticles_[i].alive=true;
				float a = 0.01f;
        if (type_ == DUST || type_ == RISINGSMOKE)
          a = 0.005f;
				velocities_[i].x=(random()-0.5f)*a;
				velocities_[i].y=(random()-0.5f)*a;
				velocities_[i].z=(random()-0.5f)*a;
        if (type_ == RISINGSMOKE)
        {
          velocities_[i].x = 0.0f;
          velocities_[i].z = 0.0f;
          velocities_[i].y += 0.02f;
          pointParticles_[i].center.size -= 0.02f * i;
        }

			}
			break;

		case GLOWFLARE:
  	  particleNo_=100;
			pointParticles_=new Storm3D_PointParticle[particleNo_];
			velocities_=new Vector[particleNo_];
		  mass_=0.001f;
  	  heat_=0.001f;
			for(i=0;i<particleNo_;i++) {

				pointParticles_[i].center.alpha=1.0f;
				pointParticles_[i].center.color.r=1.0f - i * 0.01f;
				pointParticles_[i].center.color.g=1.0f - i * 0.01f;
				pointParticles_[i].center.color.b=1.0f - i * 0.01f;

				pointParticles_[i].center.position.x=random()*2.0f-1.0f;
				pointParticles_[i].center.position.y=random()*2.0f-1.0f;
				pointParticles_[i].center.position.z=random()*2.0f-1.0f;
				
				pointParticles_[i].center.position.Normalize();
				pointParticles_[i].center.position*=(random()+0.5f)*0.5f;
				pointParticles_[i].center.position+=vector1_;

  		  pointParticles_[i].center.size=0.5f;
				
				pointParticles_[i].alive=true;
				float a = 0.01f;
        if (i == 0) a = 0;
				velocities_[i].x = vector2_.x + (random()-0.5f)*a;
				velocities_[i].y = vector2_.y + (random()-0.5f)*a;
				velocities_[i].z = vector2_.z + (random()-0.5f)*a;
        velocities_[i].y += a;
			}
      pointParticles_[0].alive = false;
  		pointParticles_[0].center.size=0.001f;
			break;

		case LASER:
			// TODO:
			break;
		
		case SWARM:
			// TODO:
			break;

		default:
			abort(); // FIXME: This shouldn't propably be abort... 
	}
}


ParticleSystem::~ParticleSystem()
{
	delete [] pointParticles_;
	delete [] lineParticles_;
	delete [] velocities_;
}


IStorm3D_Material *ParticleSystem::explosionMaterial=0;
IStorm3D_Material *ParticleSystem::sparkMaterial=0;
IStorm3D_Material *ParticleSystem::smokeMaterial=0;
IStorm3D_Material *ParticleSystem::dustMaterial=0;
IStorm3D_Material *ParticleSystem::laserMaterial=0;
IStorm3D_Material *ParticleSystem::swarmMaterial=0;
IStorm3D_Material *ParticleSystem::flameMaterial=0;
IStorm3D_Material *ParticleSystem::flareMaterial=0;

void ParticleSystem::setMaterials(IStorm3D *s3d) {
	IStorm3D_Material *c;
	if(!explosionMaterial) {
		explosionMaterial=s3d->CreateNewMaterial("ParticleSystem::explosionMaterial");
		c=explosionMaterial;
		c->SetAlphaType(IStorm3D_Material::ATYPE_ADD);

		IStorm3D_Texture *t=s3d->CreateNewTexture( "Data/Particles/explosion.jpg" );
		
		c->SetBaseTexture(t);
		c->SetColor( Color(1.0f, 0.4f, 0.1f) );
	}
  
	if(!smokeMaterial) {
		smokeMaterial=s3d->CreateNewMaterial("ParticleSystem::smokeMaterial");
		c=smokeMaterial;

		IStorm3D_Texture *t=s3d->CreateNewTexture("Data/Particles/smoke.jpg");
		c->SetAlphaType(IStorm3D_Material::ATYPE_MUL);
		c->SetBaseTexture(t);
		c->SetColor( Color(1.0f, 1.0f, 1.0f) );
	}

  if(!dustMaterial) {
		dustMaterial=s3d->CreateNewMaterial("ParticleSystem::dustMaterial");
		c=dustMaterial;

		IStorm3D_Texture *t=s3d->CreateNewTexture("Data/Particles/dust.jpg");
		//c->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
		c->SetAlphaType(IStorm3D_Material::ATYPE_MUL);
		c->SetBaseTexture(t);
		c->SetColor( Color(1.0f, 1.0f, 1.0f) );
	}

	if(!sparkMaterial) {
		sparkMaterial=s3d->CreateNewMaterial("ParticleSystem::sparkMaterial");
		c=sparkMaterial;

		IStorm3D_Texture *t=s3d->CreateNewTexture("Data/Particles/spark.jpg");

		c->SetAlphaType(IStorm3D_Material::ATYPE_ADD);
		c->SetBaseTexture(t);
		c->SetColor( Color(1.0f, 1.0f, 1.0f) );
	}

	if(!flameMaterial) {
		flameMaterial=s3d->CreateNewMaterial("ParticleSystem::flameMaterial");
		c=flameMaterial;
		c->SetAlphaType(IStorm3D_Material::ATYPE_ADD);

		IStorm3D_Texture *t=s3d->CreateNewTexture( "Data/Particles/flame.jpg" );
		
		c->SetBaseTexture(t);
		c->SetColor( Color(1.0f, 1.0f, 0.1f) );
	}

  if(!flareMaterial) {
		flareMaterial=s3d->CreateNewMaterial("ParticleSystem::flareMaterial");
		c=flareMaterial;

		IStorm3D_Texture *t=s3d->CreateNewTexture( "Data/Particles/flare.jpg" );
		
		c->SetAlphaType(IStorm3D_Material::ATYPE_ADD);
		c->SetBaseTexture(t);
		c->SetColor( Color(1.0f, 1.0f, 0.1f) );
	}
// TODO: the same for other materials
}


// Never ever pass objects by value
// The whole language was judged slow because of such misuse >;)
//	-- psd
void ParticleSystem::solver(const Vector &velocity, const Vector &forces, Vector *result, float mass, float step) 
{
	*result = velocity;
	*result += (forces/mass) * step;
}



bool ParticleSystem::stepForwards(int tick) {
  int i;
	// We default to deleting this, its particletypes responsibility to make it false
	bool returnValue=true;
	time_+=tick;

	// Don't recreate on each loop step
	//	--psd
	Vector acceleration;
	
	switch(type_) {
		case EXPLOSION:
		case EXPLOSION2:
		case EXPLOSION3:
			for(i=0;i<particleNo_;i++) {
				// Acceleration with default friction
				const float drag=6;
				//Vector acceleration;
				acceleration=velocities_[i]*-drag;
				acceleration.y+=heat_;

				solver(velocities_[i],acceleration,&velocities_[i],mass_,tick/100.0f);
				pointParticles_[i].center.position+=velocities_[i];

				pointParticles_[i].center.color.r-=0.007f;
				pointParticles_[i].center.color.g-=0.01f;
				pointParticles_[i].center.color.b-=0.01f;

				if(pointParticles_[i].center.color.b<0.01f)
					pointParticles_[i].alive=false;
				
				if(pointParticles_[i].alive) returnValue=false;

			}
			heat_*=0.0936f;

			break;
		case SPARK:
		case SPARK2:
			for(i=0;i<particleNo_;i++) {
				const float drag=9;
				//Vector acceleration;
				acceleration=velocities_[i]*-drag;
				acceleration.y-=1;
//				heat_*=.36f;

				solver(velocities_[i],acceleration,&velocities_[i],mass_,tick/100.0f);
				pointParticles_[i].center.position+=velocities_[i];

				pointParticles_[i].center.color.r-=0.005f;
				pointParticles_[i].center.color.g-=0.01f;
				pointParticles_[i].center.color.b-=0.01f;

				if(pointParticles_[i].center.color.b<0.01f)
					pointParticles_[i].alive=false;
				
				if(pointParticles_[i].alive) returnValue=false;

			}

			break;
		case FLAME:
		case ROCKETTAIL:
			// Fixed a bit
			//	-- psd

			for(i=0;i<particleNo_;i++) {
				// Acceleration with default friction
				const float drag=1;
				//Vector acceleration;
				acceleration = velocities_[i];
				acceleration *= -drag;
				acceleration.y += heat_;
				acceleration -= velocities_[i].y;
				

				solver(velocities_[i],acceleration,&velocities_[i],mass_,tick/100.0f);
				pointParticles_[i].center.position += velocities_[i];

				if (pointParticles_[i].center.size > 0.4f)
          //if (type_ == ROCKETTAIL)
	  			//	pointParticles_[i].center.size+=0.015f;
          //else
	  				pointParticles_[i].center.size+=0.025f;

				pointParticles_[i].center.color.r+=0.005f;
				pointParticles_[i].center.color.g+=0.005f;
        if (type_ == ROCKETTAIL)
  				pointParticles_[i].center.color.b-=0.02f;
        else
  				pointParticles_[i].center.color.b-=0.005f;

				if(pointParticles_[i].center.color.b<0.1f)
					pointParticles_[i].alive=false;
				
				if(pointParticles_[i].alive) returnValue=false;

			}
			heat_*=0.7995f;

			break;

    case SMOKE:
		case DUST:
		case RISINGSMOKE:
			for(i=0;i<particleNo_;i++) {
				// Acceleration with default friction
				const float drag=6;
				//Vector acceleration;
				//acceleration=velocities_[i]*-drag;
				//acceleration.y+=heat_;

				//solver(velocities_[i],acceleration,&velocities_[i],mass_,tick/100.0f);
				pointParticles_[i].center.position+=velocities_[i];

				//if(pointParticles_[i].center.alpha<0.5f)
        //{
          //pointParticles_[i].center.alpha += 0.01f;
		      //pointParticles_[i].center.color.r+=0.01f;
				  //pointParticles_[i].center.color.g+=0.01f;
				  //pointParticles_[i].center.color.b+=0.01f;
        //}

        if(pointParticles_[i].center.size>4.0f
          || type_ != RISINGSMOKE)
          pointParticles_[i].center.size += 0.01f;
        else
          pointParticles_[i].center.size += 0.0025f;

        if (type_ == DUST)
        {
				  if(pointParticles_[i].center.size>1.0f)
					  pointParticles_[i].alive=false;
        } else {
				  if(pointParticles_[i].center.size>8.0f)
					  pointParticles_[i].alive=false;

          if (type_ == RISINGSMOKE)
          {
            if (velocities_[i].x == 0.0f)
            {
              if (pointParticles_[i].center.size>2.0f)
              {
                velocities_[i].x=(random()-0.5f)*0.005f+0.004f;
   			  	    velocities_[i].z=(random()-0.5f)*0.005f+0.002f; 
              }
            }
          }
        }
				
				if(pointParticles_[i].alive) returnValue=false;

			}
			//heat_*=0.0936f;

			break;
      /*
			for(i=0;i<particleNo_;i++) {
				// Acceleration with default friction
				const float drag=5;
				//Vector acceleration;
				acceleration=velocities_[i]*-drag;
				acceleration.y+=heat_;

				solver(velocities_[i],acceleration,&velocities_[i],mass_,tick/100.0f);
				pointParticles_[i].center.position+=velocities_[i];

				pointParticles_[i].center.color.r-=0.001f;
				pointParticles_[i].center.color.g-=0.001f;
				pointParticles_[i].center.color.b-=0.001f;

				if(pointParticles_[i].center.color.b<0.01f)
					pointParticles_[i].alive=false;
				
				if(pointParticles_[i].alive) returnValue=false;

			}
			heat_*=0.09936f;
			break;
      */

		case GLOWFLARE:
		  pointParticles_[0].center.size += 0.002f;

			for(i=1;i<particleNo_;i++) {
				// Acceleration with default friction
				const float drag=6;
				//Vector acceleration;
				//acceleration=velocities_[i]*-drag;
				//acceleration.y+=heat_;

				//solver(velocities_[i],acceleration,&velocities_[i],mass_,tick/100.0f);
				pointParticles_[i].center.position+=velocities_[i];

  		  if(pointParticles_[0].center.size>10.0f)
					pointParticles_[i].alive=false;

				if(pointParticles_[i].alive) returnValue=false;

        pointParticles_[i].center.color.r -= 0.002f;
        pointParticles_[i].center.color.g -= 0.002f;
        pointParticles_[i].center.color.b -= 0.002f;
  		  if(pointParticles_[i].center.color.g < 0.01f)
        {
          pointParticles_[i].center.color.r = 1.0f;
          pointParticles_[i].center.color.g = 1.0f;
          pointParticles_[i].center.color.b = 1.0f;

				  pointParticles_[i].center.position.x=random()*2.0f-1.0f;
				  pointParticles_[i].center.position.y=random()*2.0f-1.0f;
				  pointParticles_[i].center.position.z=random()*2.0f-1.0f;
				
				  pointParticles_[i].center.position.Normalize();
				  pointParticles_[i].center.position*=(random()+0.5f)*0.5f;
				  pointParticles_[i].center.position+=
            pointParticles_[0].center.position;

				  velocities_[i].x = velocities_[0].x + (random()-0.5f)*0.01f;
				  velocities_[i].y = velocities_[0].y + (random()-0.5f)*0.01f;
				  velocities_[i].z = velocities_[0].z + (random()-0.5f)*0.01f;
          velocities_[i].y += 0.01f;
        }

			}

			break;

		default:
			abort(); /* FIXME: Shouldn't propably abort */
	}

	return returnValue;

}



bool ParticleSystem::render() {
	switch(type_) {
		case EXPLOSION:
		case EXPLOSION2:
		case EXPLOSION3:
			particleSystem_->RenderParticles(explosionMaterial,pointParticles_,particleNo_);
			break;
		case SPARK:
		case SPARK2:
			particleSystem_->RenderParticles(sparkMaterial,pointParticles_,particleNo_);
			break;
		case FLAME:
		case ROCKETTAIL:
			particleSystem_->RenderParticles(flameMaterial,pointParticles_,particleNo_);
			break;
		case SMOKE:
		case RISINGSMOKE:
			particleSystem_->RenderParticles(smokeMaterial,pointParticles_,particleNo_);
			break;
		case DUST:
			particleSystem_->RenderParticles(dustMaterial,pointParticles_,particleNo_);
			break;
		case GLOWFLARE:
			particleSystem_->RenderParticles(flareMaterial,pointParticles_,particleNo_);
			break;
		default:
			abort();
	}
	return true;
}



} // namespace ui

/* EOF */


#ifndef GAMECAMERA_H
#define GAMECAMERA_H

// half a second interpolation 
#define CAM_DEFAULT_TIME_TO_INTERPOLATE 0.5f

#include "../game/gamedefs.h"
#include <DatatypeDef.h>

class IStorm3D_Scene;

namespace game
{
  class GameMap;
	class WaterManager;
}


namespace ui
{

  class GameController;
	class IGameCameraBoundary;

	class GameCameraImplData;


  class GameCamera
  {
  public:
    enum CAMERA_MODE
    {
      CAMERA_MODE_INVALID = 0,
      CAMERA_MODE_ZOOM_CENTRIC = 1,
      CAMERA_MODE_CAMERA_CENTRIC = 2,
      CAMERA_MODE_TARGET_CENTRIC = 3,
      CAMERA_MODE_FLYING = 4
    };
    enum CAMERA_MOVE
    {
      CAMERA_MOVE_INVALID = 0,
      CAMERA_MOVE_FORWARD = 1,
      CAMERA_MOVE_BACKWARD = 2,
      CAMERA_MOVE_LEFT = 3,
      CAMERA_MOVE_RIGHT = 4,
      CAMERA_MOVE_ROTATE_LEFT = 5,
      CAMERA_MOVE_ROTATE_RIGHT = 6,
      CAMERA_MOVE_ORBIT_LEFT = 7,
      CAMERA_MOVE_ORBIT_RIGHT = 8,
      CAMERA_MOVE_ZOOM_IN = 9,
      CAMERA_MOVE_ZOOM_OUT = 10,
      CAMERA_MOVE_UP = 11,
      CAMERA_MOVE_DOWN = 12,
      CAMERA_MOVE_ROTATE_UP = 13,
      CAMERA_MOVE_ROTATE_DOWN = 14,
      CAMERA_MOVE_ZOOM_NEXT = 15,
      CAMERA_MOVE_FOV_IN = 16,
      CAMERA_MOVE_FOV_OUT = 17
    };

    static CAMERA_MOVE getCameraMoveByName(char *name);

    static CAMERA_MODE getCameraModeByName(char *name);
    
    GameCamera(IStorm3D_Scene *scene, game::GameMap *gameMap,
      GameController **gameControllers);
    ~GameCamera();

    void setWaterManager(game::WaterManager *waterManager);

    void setMode(CAMERA_MODE cameraMode);

    CAMERA_MODE getMode();

    void setBoundaries(IGameCameraBoundary *boundaries);

    void setPositionNear(float x, float y);

    VC3 getPosition();

    VC3 getActualInterpolatedPosition();

    void setPosition(const VC3 &position);

    void rotateAroundPosition(VC3 &position, int angle);

		// now handled by directly checking the option value
    //void setInvertLook(bool inverted);

    void setDisableUserMovement(bool disabled);
    bool isUserMovementDisabled();

    void setFollowingUnit(bool following);
    void setFollowingUnitBehind(bool followBehind);
    bool isFollowingUnit();
    bool isFollowingUnitBehind();

    void setFollowPosition(float x, float y);

    void setFirstPersonMode(bool firstPersonMode);
    bool isFirstPersonMode();

    void setFirstPersonPosition(float personX, float personY, float scrolledX, float scrolledY, float height, float alphaAngle, float betaAngle);

    void setMovement(CAMERA_MOVE moveType, bool moving);
    //void followUnit(game::Unit *unit);

    void setDestination(float x, float y);

    void doMovement(float delta);

	  void setShakeEffect(int amount, int totalTime, const VC3 &position);

    float getAngleY();

    bool isThirdPersonView();
    void setThirdPersonView(bool thirdPersonView);

    void setAngleY(float angle);

    void setBetaAngle(float angle);
    float getBetaAngle();

    void setMinBetaAngle(float angle);
    void setMaxBetaAngle(float angle);

    void setFOV(float fov);
    float getFOV();

    static void setFOVFactor(float factor);

		void setHeight(float height);

    // slowly moves this camera to current position from the given
    // camera's position.
    void interpolateFrom(GameCamera *camera, float seconds = CAM_DEFAULT_TIME_TO_INTERPOLATE);

		void copyFrom(GameCamera *camera);

		void applyToSceneAbsolute(VC3 position, VC3 target, VC3 up, float FOV);

		void applyToScene();

		void resetForScene();

		void setScrollPosition(const VC3 &position, int clientNumber);

		VC3 getScrollPosition(int clientNumber);

		void setSmoothCameraInterpolation(bool smoothInterpolation);

		void zoomTo(float zoom);

		// NOTE: this method should not be here, it is not part of camera's implementation :)
		void setGameCameraMode(bool aimUpward);
		bool getGameCameraMode();

		// NOTE: this method should not be here, it is not part of camera's implementation :)
		void setGameCameraYAxisLock(bool locked);
		bool getGameCameraYAxisLock();

		VC3 getTargetPosition() { return VC3(targetX, targetHeight, targetY); }

		void setUpVector(const VC3 &upVector);

		VC3 getTargetOffset() { return targetOffset; }
		VC3 getPositionOffset() { return positionOffset; }
		void setTargetOffset(const VC3 &targetOffset) { this->targetOffset = targetOffset; }
		void setPositionOffset(const VC3 &positionOffset) { this->positionOffset = positionOffset; }

		void setDirectControlsEnabled(bool enabled) { this->directControlsEnabled = enabled; }

  private:
    IStorm3D_Scene *scene;
    game::GameMap *gameMap;
    GameController **gameControllers;
    game::WaterManager *waterManager;

    IGameCameraBoundary *boundaries;

    GameCamera *interpolateCamera;
    float interpolateTime;
    float interpolateDuration;

		VC3 interpolatedPosition;
		VC3 interpolatedPositionOffset;
		VC3 interpolatedTargetOffset;

    CAMERA_MODE cameraMode;
    //bool invertLook;

    float alphaAngle;
    float betaAngle;

    float minBetaAngle;
    float maxBetaAngle;

    float cameraVelocity;
    float cameraX;
    float cameraY;
    float cameraHeight;

    float targetX;
    float targetY;
    float targetHeight;

    float destinationX;
    float destinationY;
    bool toDestination;
    float destinationTime;
    float sourceX;
    float sourceY;

    float followX;
    float followY;

    float zoom;
    float zooming_to;

		float fov;

    bool move_orbitleft;
    bool move_orbitright;
    bool move_rotleft;
    bool move_rotright;
    bool move_rotup;
    bool move_rotdown;
    bool move_goup;
    bool move_godown;
    bool move_fwd;
    bool move_back;
    bool move_left;
    bool move_right;
    bool move_zoom_in;
    bool move_zoom_out;
    bool move_fov_in;
    bool move_fov_out;

    bool key_move_orbitleft;
    bool key_move_orbitright;
    bool key_move_rotleft;
    bool key_move_rotright;
    bool key_move_rotup;
    bool key_move_rotdown;
    bool key_move_goup;
    bool key_move_godown;
    bool key_move_fwd;
    bool key_move_back;
    bool key_move_left;
    bool key_move_right;
    bool key_move_zoom_in;
    bool key_move_zoom_out;
    bool key_move_fov_in;
    bool key_move_fov_out;

	 bool forceMapView;
	 float screenAspect;
	 int mapviewAreaX, mapviewAreaY;
	 int mapviewWidth, mapviewHeight;

    //game::Unit *followingUnit;
    bool followingUnit;
    bool followBehind;

    bool disabledUserMovement;

    float minZoom;
    float maxZoom;
    float middleZoom; 
    float targDist;

    bool firstPersonMode;
    bool thirdPerson;

		VC3 scrollPosition[MAX_PLAYERS_PER_CLIENT];

		GameCameraImplData *implData;

		bool smoothInterpolation;

		VC3 positionOffset;
		VC3 targetOffset;

		bool directControlsEnabled;

		static float fovFactor;

  public:
    inline void setZoom(float zoom) { this->zoom = zoom; this->zooming_to = zoom; }
    inline float getZoom() { return this->zoom; }
		inline float getZoomingTo() { return this->zooming_to; }
    inline void setTargetDistance(float targDist) { this->targDist = targDist; }
    inline float getTargetDistance() { return this->targDist; }
    void restoreDefaultTargetDistance();
    inline void setMinZoom(float minZoom) { this->minZoom = minZoom; }
    inline void setMaxZoom(float maxZoom) { this->maxZoom = maxZoom; }
    inline void setMiddleZoom(float middleZoom) { this->middleZoom = middleZoom; }

	 // Used by setMapView script command.
	 inline void setForceMapView( bool a_forceMapView, int a_areaX = 0, int a_areaY = 0, int width = 0, int height = 0, float a_screenAspect = 1024.0f / 768 ) { forceMapView = a_forceMapView; screenAspect = a_screenAspect; mapviewAreaX = a_areaX; mapviewAreaY = a_areaY; mapviewWidth = width; mapviewHeight = height; };
  };

}

#endif

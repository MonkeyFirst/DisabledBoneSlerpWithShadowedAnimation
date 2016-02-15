#include "Urho3DAll.h"
#include "GameScene.h"
#include "Player.h"
#include "PlayerCamera.h"
#include "DebugUI.h"

using namespace Urho3D;

class MyApp : public Application
{
public:
    SharedPtr<GameScene> gameScene;
    SharedPtr<Player> player;
    SharedPtr<DebugUI> debugUI;
    SharedPtr<Node> SunNode;

    SharedPtr<Node> animNode;
    SharedPtr<AnimatedModel> animModel;
    SharedPtr<AnimationState> animState;
    Skeleton sk;
    Bone* bone;
    bool goSlerpBones;
    float goSlerpTime;

    int fps = 0;
    int fpsCounter = 0;
    float fpsTimer = 0;

    MyApp(Context* context) :
        Application(context)
    {
        PlayerCamera::RegisterObject(context);
    }

	virtual void Setup()
	{
		// Called before engine initialization. engineParameters_ member variable can be modified here
		engineParameters_["WindowTitle"] = "Test";
		engineParameters_["FullScreen"] = false;
		engineParameters_["Headless"] = false;
		engineParameters_["WindowWidth"] = 1235;
		engineParameters_["WindowHeight"] = 678;
		engineParameters_["LogName"] = GetTypeName() + ".log";
		engineParameters_["VSync"] = false;
		engineParameters_["FrameLimiter"] = false;
		engineParameters_["WorkerThreads"] = true;
		engineParameters_["RenderPath"] = "CoreData/RenderPaths/Forward.xml";
		engineParameters_["Sound"] = false;
    }

	virtual void Start()
	{
		// Called after engine initialization. Setup application & subscribe to events here
		SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MyApp, HandleKeyDown));
		SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MyApp, HandleUpdate));
		SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(MyApp, HandleFixedUpdate));
		SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(MyApp, HandleMouseButtonDown));
		SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(MyApp, HandlePostRenderUpdate));
        SubscribeToEvent(E_ENDFRAME, URHO3D_HANDLER(MyApp, HandleEndFrame));

        

		ResourceCache* cache = context_->GetSubsystem<ResourceCache>();

		gameScene = new GameScene(context_);
		gameScene->Create("Scene1.xml");

		player = new Player(context_);
		player->Create(gameScene, "PlayerNode");

		debugUI = new DebugUI(context_);
		debugUI->CreateDebugUI();

        debugUI->ToggleDebugUI();

        animNode = gameScene->scene->GetChild("anim", true);
        animModel = animNode->GetComponent<AnimatedModel>();
        int i = 0;
        animState = animModel->GetAnimationState(i);
        
        bone = animModel->GetSkeleton().GetBone("Bone"); // get bone
        bone->animated_ = false; // turn-off animation
        bone->shadowedAnimated_ = true; // turn-on shadowed animation
        
        goSlerpBones = false;
        goSlerpTime = 0.0f;
        
    }

    virtual void Stop()
    {
        // Perform optional cleanup after main loop has terminated

    }

	void HandleMouseButtonDown(StringHash eventType, VariantMap& eventData)
	{
		using namespace MouseButtonDown;
		int button = eventData[P_BUTTON].GetInt();

		if (button == MOUSEB_LEFT) 
		{

		}
	}

    void HandleKeyDown(StringHash eventType, VariantMap& eventData)
    {
        using namespace KeyDown;
		ResourceCache* cache = context_->GetSubsystem<ResourceCache>();

        // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESC)
            engine_->Exit();

        if (key == KEY_F2)
            debugUI->ToggleDebugUI();

        if (key == KEY_F3) // start slerp
        {
            goSlerpBones = true;
            goSlerpTime = 0.0f;
            bone->animated_ = false;
        }

        if (key == KEY_F4) // stop animation for bone
        {
            goSlerpBones = false;
            goSlerpTime = 0.0f;
            bone->animated_ = false;
        }

    }

	void HandleUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace Update;
		float timeStep = eventData[P_TIMESTEP].GetFloat();

        fpsTimer += timeStep;
        fpsCounter++;
        if (fpsTimer >= 1.0f)
        {
            fps = fpsCounter;
            fpsCounter = 0;
            fpsTimer = 0;
            // и тут выводим fps куда надо
        }

        animState->AddTime(timeStep);

        const float timeToReachAnimationTrack = 2.0f; // sec

        if (goSlerpBones) 
        {   
            if (goSlerpTime < 0.2f)
            {
                goSlerpTime += ((0.2f / timeToReachAnimationTrack) * timeStep);
                bone->node_->SetRotation(bone->node_->GetRotation().Slerp(bone->shadowedBoneTransform.rotation, goSlerpTime));
            }
            else 
            {
                bone->animated_ = true;
                goSlerpTime = 0.0f;
                goSlerpBones = false;
            }
        }
                
        debugUI->debugHud->SetAppStats("shadowed bone position", String(bone->shadowedBoneTransform.position));
        debugUI->debugHud->SetAppStats("shadowed bone rotation", String(bone->shadowedBoneTransform.rotation));
        debugUI->debugHud->SetAppStats("shadowed bone scale", String(bone->shadowedBoneTransform.scale));
        debugUI->debugHud->SetAppStats("shadowed do slerp", String(goSlerpBones));
        debugUI->debugHud->SetAppStats("shadowed slerp time(normalized)", String(goSlerpTime));
        debugUI->debugHud->SetAppStats("is bone.animated_", String(bone->animated_));
        debugUI->debugHud->SetAppStats("is bone.shadowedAnimated_", String(bone->shadowedAnimated_));

	}

    void HandleEndFrame(StringHash eventType, VariantMap& eventData)
    {
        using namespace EndFrame;

        static Graphics* g = GetSubsystem<Graphics>();

        g->SetWindowTitle("FPS= " + String(fps));

    }

	void HandleFixedUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace Update;
		float timeStep = eventData[P_TIMESTEP].GetFloat();	
	}

	void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
	{
        using namespace PostRenderUpdate;
        float timeStep = eventData[P_TIMESTEP].GetFloat();
	}
};

URHO3D_DEFINE_APPLICATION_MAIN(MyApp)
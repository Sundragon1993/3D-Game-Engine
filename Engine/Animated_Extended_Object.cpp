#include "Engine.h"

Animated_Extended_Object::Animated_Extended_Object(char *meshName, char *meshPath, unsigned long type) : SceneObject(type, meshName, meshPath, false)
{
	if (GetMesh() != NULL)
		GetMesh()->CloneAnimationController(&m_animationController);
	else
		m_animationController = NULL;

	if (m_animationController != NULL)
	{
		m_animationController->SetTrackSpeed(0, 1.0f);
		m_animationController->SetTrackSpeed(1, 1.0f);
	}

	m_currentTrack = 0;
	m_currentTime = 0.0f;
}

Animated_Extended_Object::~Animated_Extended_Object()
{
	SAFE_RELEASE(m_animationController);
}

void Animated_Extended_Object::Update(float elapsed, bool addVelocity)
{
	SceneObject::Update(elapsed, addVelocity);

	if (m_animationController)
	{
		m_animationController->AdvanceTime(elapsed, this);

		m_currentTime += elapsed;
	}

	// Update the mesh.
	if (GetMesh() != NULL)
		GetMesh()->Update();
}


void Animated_Extended_Object::PlayAnimation(unsigned int animation, float transitionTime, bool loop)
{
	if (m_animationController == NULL)
		return;

	if (transitionTime <= 0.0f)
		transitionTime = 0.000001f;

	unsigned int newTrack = (m_currentTrack == 0 ? 1 : 0);

	ID3DXAnimationSet *as;
	m_animationController->GetAnimationSet(animation, &as);

	m_animationController->SetTrackAnimationSet(newTrack, as);

	m_animationController->UnkeyAllTrackEvents(m_currentTrack);
	m_animationController->UnkeyAllTrackEvents(newTrack);

	if (loop == true)
	{
		// Transition the new track in over the specified transition time period.
		m_animationController->KeyTrackEnable(m_currentTrack, false, m_currentTime + transitionTime);
		m_animationController->KeyTrackWeight(m_currentTrack, 0.0f, m_currentTime, transitionTime, D3DXTRANSITION_LINEAR);
		m_animationController->SetTrackEnable(newTrack, true);
		m_animationController->KeyTrackWeight(newTrack, 1.0f, m_currentTime, transitionTime, D3DXTRANSITION_LINEAR);
	}
	else
	{
		// Stop the current track, and start the new track without transitioning.
		m_animationController->SetTrackEnable(m_currentTrack, false);
		m_animationController->SetTrackWeight(m_currentTrack, 0.0f);
		m_animationController->SetTrackEnable(newTrack, true);
		m_animationController->SetTrackWeight(newTrack, 1.0f);
		m_animationController->SetTrackPosition(newTrack, 0.0f);
		m_animationController->KeyTrackEnable(newTrack, false, m_currentTime + as->GetPeriod());
	}

	// Release the pointer to the animation set.
	as->Release();

	// The new track is now the current track.
	m_currentTrack = newTrack;
}

ID3DXAnimationController *Animated_Extended_Object::GetAnimationController()
{
	return m_animationController;
}

HRESULT CALLBACK Animated_Extended_Object::HandleCallback(THIS_ UINT Track, LPVOID pCallbackData)
{
	return S_OK;
}

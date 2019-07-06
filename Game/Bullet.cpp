#include "Main.h"
Bullet::Bullet(SceneObject *owner, D3DXVECTOR3 translation, D3DXVECTOR3 direction, float velocity, float range, float damage, char*meshName, char* meshPath, unsigned long type) :SceneObject(type, meshName, meshPath, false)
{
	this->m_owner = owner;

	m_hitResult = new RayIntersectionResult;

	m_totalDistance = 0.0f;

	m_expired = false;

	m_translation = translation;
	m_direction = direction;
	m_velocity = velocity;
	m_range = range;
	m_damage = damage;
}

Bullet::~Bullet()
{
	SAFE_DELETE(m_hitResult);
}

void Bullet::Update(float elapsed)
{
	// Clear the hit result.
	m_hitResult->material = NULL;
	m_hitResult->distance = 0.0f;
	m_hitResult->hitObject = NULL;

	// Cast a ray through the scene to see if the bullet might hit something.
	if (g_engine->GetSceneManager()->RayIntersectScene(m_hitResult, m_translation, m_direction, true, m_owner, true) == true)
	{
		// Ensure the hit distance is within the velocity of the bullet.
		if (m_hitResult->distance <= m_velocity * elapsed)
		{
			// Ensure the bullet will not go beyond its range.
			m_totalDistance += m_hitResult->distance;
			if (m_totalDistance > m_range)
			{
				// Clear the hit result.
				m_hitResult->material = NULL;
				m_hitResult->distance = 0.0f;
				m_hitResult->hitObject = NULL;

				// Indicate that the bullet has expired.
				m_expired = true;

				return;
			}

			m_expired = true;

			if (g_engine->GetNetwork()->IsHost() == false)
				return;

			if (m_hitResult->hitObject == NULL)
				return;

			if (m_hitResult->hitObject->GetType() == TYPE_PLAYER_OBJECT)
				((PlayerObject*)m_hitResult->hitObject)->Hurt(m_damage, (PlayerObject*)m_owner);

			return;
		}
		else
		{
			// Clear the hit result.
			m_hitResult->material = NULL;
			m_hitResult->distance = 0.0f;
			m_hitResult->hitObject = NULL;
		}
	}

	// Expire the bullet if it will move beyond its range.
	m_totalDistance += m_velocity * elapsed;
	if (m_totalDistance > m_range)
	{
		m_expired = true;

		return;
	}

	// Move the bullet.
	m_translation += m_direction * (m_velocity * elapsed);
}

bool Bullet::IsExpired()
{
	return m_expired;
}

BulletManager::BulletManager()
{
	// Create the linked list of bullets.
	m_bullets = new LinkedList< Bullet >;
}

BulletManager::~BulletManager()
{
	SAFE_DELETE(m_bullets);
}

void BulletManager::Update(float elapsed)
{
	// Go through the list of bullets.
	Bullet *remove = NULL;
	Bullet *bullet = m_bullets->GetFirst();
	while (bullet != NULL)
	{
		// Check if the bullet has expired. If not, allow it to update.
		if (bullet->IsExpired() == true)
			remove = bullet;
		else
			bullet->Update(elapsed);

		// Go to the next bullet.
		bullet = m_bullets->GetNext(bullet);

		// If the bullet did expire, then remove it now.
		if (remove != NULL)
			m_bullets->Remove(&remove);
	}
}

//-----------------------------------------------------------------------------
// Adds a new bullet to the bullet manager.
// Need fix for bullet 15/2/2017
//-----------------------------------------------------------------------------
void BulletManager::AddBullet(SceneObject *owner, D3DXVECTOR3 translation, D3DXVECTOR3 direction, float velocity, float range, float damage)
{
	m_bullets->Add(new Bullet(owner, translation, direction, velocity, range, damage, "bullet", "./Game/Assets/bullet.x"));
}
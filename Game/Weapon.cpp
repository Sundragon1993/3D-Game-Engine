#include "Main.h"

Weapon::Weapon(Script *script, D3DXVECTOR3 viewWeaponOffset)
{
	// Get the name of the weapon.
	m_name = new char[strlen(script->GetStringData("name")) + 1];
	strcpy(m_name, script->GetStringData("name"));

	m_valid = true;

	m_viewWeaponOffset = viewWeaponOffset;

	m_lastViewPointY = m_offsetViewPointY = 0.0f;

	m_useViewWeapon = false;

	// Get all the details about the weapon's firing capabilities.
	m_fireTimer = m_rof = *script->GetFloatData("rate_of_fire");
	m_velocity = *script->GetFloatData("muzzle_velocity") / g_engine->GetScale();
	m_damage = *script->GetFloatData("max_damage");
	m_range = *script->GetFloatData("range") / g_engine->GetScale();

	// Create an object for the weapon on the player's body.
	m_bodyWeapon = new SceneObject(TYPE_SCENE_OBJECT, script->GetStringData("mesh"), script->GetStringData("mesh_path"));

	// Create an object for the weapon in the player's view.
	m_viewWeapon = new Animated_Extended_Object(script->GetStringData("mesh_hands"), script->GetStringData("mesh_path"));

	// Create the list of shot flashes for this weapon.
	m_flashes = new LinkedList< Material >;
	char flash[16] = { "flash0" };
	while (script->GetStringData(flash) != NULL)
	{
		m_flashes->Add(new Material(script->GetStringData(flash), script->GetStringData("flash_path")));
		sprintf(flash, "flash%d", m_flashes->GetTotalElements());
	}
	m_displayFlash = false;

	m_shotSound = new Sound(script->GetStringData("sound"));
	m_shotAudioPath = new AudioPath3D;
}

Weapon::~Weapon()
{
	SAFE_DELETE(m_name);

	SAFE_DELETE(m_bodyWeapon);
	SAFE_DELETE(m_viewWeapon);

	SAFE_DELETE(m_flashes);

	SAFE_DELETE(m_shotSound);
	SAFE_DELETE(m_shotAudioPath);
}

void Weapon::Update(float elapsed, bool fire, SceneObject *parent, float viewPointY)
{
	// Update weapon, using either the 3rd person or the 1st person object.
	if (m_useViewWeapon == false)
		m_bodyWeapon->Update(elapsed, false);
	else
	{
		// Calculate the offset in model space.
		D3DXVECTOR3 offset;
		D3DXVec3TransformCoord(&offset, &m_viewWeaponOffset, parent->GetRotationMatrix());

		if (m_lastViewPointY == 0.0f)
			m_lastViewPointY = viewPointY;

		// When the player's mesh animates, the head moves, which causes the
		// view point to move. Take this movement into account so that the
		// weapon does not fly all over the place on the screen.
		m_offsetViewPointY += (viewPointY - m_lastViewPointY) * 0.8f;
		offset.y += m_offsetViewPointY;
		m_lastViewPointY = viewPointY;

		// Position the weapon correctly, then update it.
		m_viewWeapon->SetTranslation(parent->GetTranslation() + offset);
		m_viewWeapon->SetRotation(parent->GetRotation());
		m_viewWeapon->AddRotation(((PlayerObject*)parent)->GetViewTilt(), 0.0f, 0.0f);
		m_viewWeapon->Update(elapsed, false);
	}

	m_fireTimer += elapsed;

	if (fire == false)
		return;

	if (m_fireTimer > m_rof)
	{
		m_displayFlash = true;

		if (m_useViewWeapon == true)
		{
			m_muzzelPoint = m_viewWeapon->GetMesh()->GetReferencePoint("rp_sight")->GetTranslation();

			D3DXMATRIX rotationXMatrix;
			D3DXMatrixRotationX(&rotationXMatrix, ((PlayerObject*)parent)->GetViewTilt());

			D3DXMATRIX combinedRotation;
			D3DXMatrixMultiply(&combinedRotation, &rotationXMatrix, parent->GetRotationMatrix());

			D3DXVec3TransformCoord(&m_muzzelPoint, &m_muzzelPoint, &combinedRotation);

			m_muzzelPoint += m_viewWeapon->GetTranslation();

			m_shotAudioPath->SetMode(DS3DMODE_HEADRELATIVE);

			m_shotAudioPath->SetPosition(m_viewWeapon->GetMesh()->GetReferencePoint("rp_sight")->GetTranslation());
			m_shotAudioPath->SetVelocity(parent->GetVelocity());
		}
		else
		{
			// Calculate the muzzel point of the weapon in world space
			// using the 3rd person view weapon object.
			m_muzzelPoint = m_bodyWeapon->GetMesh()->GetReferencePoint("rp_sight")->GetTranslation();

			Frame *attach = ((PlayerObject*)parent)->GetMesh()->GetReferencePoint("rp_weapon_attach");
			D3DXMATRIX transform;
			D3DXMatrixMultiply(&transform, &attach->finalTransformationMatrix, parent->GetWorldMatrix());

			D3DXVec3TransformCoord(&m_muzzelPoint, &m_muzzelPoint, &transform);

			m_shotAudioPath->SetMode(DS3DMODE_NORMAL);

			m_shotAudioPath->SetPosition(m_muzzelPoint);
			m_shotAudioPath->SetVelocity(parent->GetVelocity());
		}

		m_shotAudioPath->Play(m_shotSound->GetSegment());

		g_game->GetBulletManager()->AddBullet(parent, ((PlayerObject*)parent)->GetEyePoint(), ((PlayerObject*)parent)->GetForwardVector(), m_velocity, m_range, m_damage);

		m_fireTimer = 0.0f;
	}
}

void Weapon::Render(SceneObject *parent)
{
	if (m_useViewWeapon == true)
		m_viewWeapon->Render();
	else
	{
		// Get the translation (world space) to render the 3rd person weapon.
		Frame *attach = ((PlayerObject*)parent)->GetMesh()->GetReferencePoint("rp_weapon_attach");
		D3DXMATRIX transform;
		D3DXMatrixMultiply(&transform, &attach->finalTransformationMatrix, parent->GetWorldMatrix());

		m_bodyWeapon->Render(&transform);
	}

	if (m_displayFlash == true)
	{
		Material *material = m_flashes->GetRandom();

		D3DXMATRIX view, world;
		D3DXMatrixIdentity(&world);
		g_engine->GetDevice()->GetTransform(D3DTS_VIEW, &view);
		g_engine->GetSprite()->SetWorldViewLH(&world, &view);
		g_engine->GetSprite()->Begin(D3DXSPRITE_BILLBOARD | D3DXSPRITE_ALPHABLEND);
		g_engine->GetSprite()->Draw(material->GetTexture(), NULL, &D3DXVECTOR3(material->GetWidth() / 2.0f, material->GetHeight() / 2.0f, 0.0f), &m_muzzelPoint, 0xFFFFFFFF);
		g_engine->GetSprite()->End();

		m_displayFlash = false;
	}
}

void Weapon::RaiseLowerWeapon(float elapsed, SceneObject *parent, float move)
{
	D3DXVECTOR3 offset;
	D3DXVec3TransformCoord(&offset, &m_viewWeaponOffset, parent->GetRotationMatrix());

	D3DXVECTOR3 up;
	D3DXVec3Cross(&up, &parent->GetForwardVector(), &parent->GetRightVector());

	offset += up*move;

	m_viewWeapon->SetTranslation(parent->GetTranslation() + offset);
	m_viewWeapon->SetRotation(parent->GetRotation());
	m_viewWeapon->AddRotation(((PlayerObject*)parent)->GetViewTilt(), 0.0f, 0.0f);
	m_viewWeapon->Update(elapsed, false);
}

void Weapon::UseViewWeapon(bool use)
{
	m_useViewWeapon = use;
}

char *Weapon::GetName()
{
	return m_name;
}

void Weapon::SetValid(bool valid)
{
	m_valid = valid;
}

bool Weapon::GetValid()
{
	return m_valid;
}
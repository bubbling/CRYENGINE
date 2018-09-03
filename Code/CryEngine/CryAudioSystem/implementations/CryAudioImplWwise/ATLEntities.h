// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "GlobalData.h"
#include "Common.h"

#include <SharedAudioData.h>
#include <IAudioImpl.h>
#include <PoolObject.h>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>

namespace CryAudio
{
namespace Impl
{
namespace Wwise
{
class CEvent;

enum class EObjectFlags : EnumFlagsType
{
	None                  = 0,
	MovingOrDecaying      = BIT(0),
	TrackAbsoluteVelocity = BIT(1),
	TrackRelativeVelocity = BIT(2),
	UpdateVirtualStates   = BIT(3),
};
CRY_CREATE_ENUM_FLAG_OPERATORS(EObjectFlags);

class CObject final : public IObject, public CPoolObject<CObject, stl::PSyncNone>
{
public:

	using AuxSendValues = std::vector<AkAuxSendValue>;

	explicit CObject(AkGameObjectID const id, CObjectTransformation const& transformation);
	virtual ~CObject() override;

	CObject() = delete;
	CObject(CObject const&) = delete;
	CObject(CObject&&) = delete;
	CObject& operator=(CObject const&) = delete;
	CObject& operator=(CObject&&) = delete;

	// CryAudio::Impl::IObject
	virtual void                         Update(float const deltaTime) override;
	virtual void                         SetTransformation(CObjectTransformation const& transformation) override;
	virtual CObjectTransformation const& GetTransformation() const override { return m_transformation; }
	virtual void                         SetEnvironment(IEnvironment const* const pIEnvironment, float const amount) override;
	virtual void                         SetParameter(IParameter const* const pIParameter, float const value) override;
	virtual void                         SetSwitchState(ISwitchState const* const pISwitchState) override;
	virtual void                         SetObstructionOcclusion(float const obstruction, float const occlusion) override;
	virtual void                         SetOcclusionType(EOcclusionType const occlusionType) override;
	virtual ERequestStatus               ExecuteTrigger(ITrigger const* const pITrigger, IEvent* const pIEvent) override;
	virtual void                         StopAllTriggers() override;
	virtual ERequestStatus               PlayFile(IStandaloneFile* const pIStandaloneFile) override;
	virtual ERequestStatus               StopFile(IStandaloneFile* const pIStandaloneFile) override;
	virtual ERequestStatus               SetName(char const* const szName) override;
	virtual void                         ToggleFunctionality(EObjectFunctionality const type, bool const enable) override;

	// Below data is only used when INCLUDE_WWISE_IMPL_PRODUCTION_CODE is defined!
	virtual void DrawDebugInfo(IRenderAuxGeom& auxGeom, float const posX, float posY, char const* const szTextFilter) override;
	// ~CryAudio::Impl::IObject

	void RemoveEvent(CEvent* const pEvent);

	AkGameObjectID const m_id;

private:

	void PostEnvironmentAmounts();
	void UpdateVelocities(float const deltaTime);
	void TryToSetRelativeVelocity(float const relativeVelocity);

	std::vector<CEvent*>  m_events;
	bool                  m_needsToUpdateEnvironments;
	bool                  m_needsToUpdateVirtualStates;
	AuxSendValues         m_auxSendValues;

	EObjectFlags          m_flags;
	float                 m_distanceToListener;
	float                 m_previousRelativeVelocity;
	float                 m_previousAbsoluteVelocity;
	CObjectTransformation m_transformation;
	Vec3                  m_position;
	Vec3                  m_previousPosition;
	Vec3                  m_velocity;

#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	std::map<char const* const, float> m_parameterInfo;
#endif  // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
};

class CListener final : public IListener
{
public:

	CListener() = delete;
	CListener(CListener const&) = delete;
	CListener(CListener&&) = delete;
	CListener& operator=(CListener const&) = delete;
	CListener& operator=(CListener&&) = delete;

	explicit CListener(CObjectTransformation const& transformation, AkGameObjectID const id);

	// CryAudio::Impl::IListener
	virtual void                         Update(float const deltaTime) override;
	virtual void                         SetName(char const* const szName) override;
	virtual void                         SetTransformation(CObjectTransformation const& transformation) override;
	virtual CObjectTransformation const& GetTransformation() const override { return m_transformation; }
	// ~CryAudio::Impl::IListener

	AkGameObjectID GetId() const       { return m_id; }
	bool           HasMoved() const    { return m_hasMoved; }
	Vec3 const&    GetPosition() const { return m_position; }
	Vec3 const&    GetVelocity() const { return m_velocity; }

#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	char const* GetName() const { return m_name.c_str(); }
#endif  // INCLUDE_WWISE_IMPL_PRODUCTION_CODE

private:

	AkGameObjectID const  m_id;
	bool                  m_hasMoved;
	bool                  m_isMovingOrDecaying;
	Vec3                  m_velocity;
	Vec3                  m_position;
	Vec3                  m_previousPosition;
	CObjectTransformation m_transformation;

#if defined(INCLUDE_WWISE_IMPL_PRODUCTION_CODE)
	CryFixedStringT<MaxObjectNameLength> m_name;
#endif  // INCLUDE_WWISE_IMPL_PRODUCTION_CODE
};

class CTrigger final : public ITrigger
{
public:

	explicit CTrigger(AkUniqueID const id, float const maxAttenuation)
		: m_id(id)
		, m_maxAttenuation(maxAttenuation)
	{}

	CTrigger(CTrigger const&) = delete;
	CTrigger(CTrigger&&) = delete;
	CTrigger& operator=(CTrigger const&) = delete;
	CTrigger& operator=(CTrigger&&) = delete;

	// CryAudio::Impl::ITrigger
	virtual ERequestStatus Load() const override;
	virtual ERequestStatus Unload() const override;
	virtual ERequestStatus LoadAsync(IEvent* const pIEvent) const override;
	virtual ERequestStatus UnloadAsync(IEvent* const pIEvent) const override;
	// ~CryAudio::Impl::ITrigger

	AkUniqueID const m_id;
	float const      m_maxAttenuation;

private:

	ERequestStatus SetLoaded(bool const bLoad) const;
	ERequestStatus SetLoadedAsync(IEvent* const pIEvent, bool const bLoad) const;
};

struct SParameter final : public IParameter
{
	explicit SParameter(AkRtpcID const id_, float const mult_, float const shift_)
		: mult(mult_)
		, shift(shift_)
		, id(id_)
	{}

	SParameter(SParameter const&) = delete;
	SParameter(SParameter&&) = delete;
	SParameter& operator=(SParameter const&) = delete;
	SParameter& operator=(SParameter&&) = delete;

	float const    mult;
	float const    shift;
	AkRtpcID const id;
};

enum class ESwitchType : EnumFlagsType
{
	None,
	Rtpc,
	StateGroup,
	SwitchGroup,
};

struct SSwitchState final : public ISwitchState
{
	explicit SSwitchState(
		ESwitchType const type_,
		AkUInt32 const stateOrSwitchGroupId_,
		AkUInt32 const stateOrSwitchId_,
		float const rtpcValue_ = s_defaultStateValue)
		: type(type_)
		, stateOrSwitchGroupId(stateOrSwitchGroupId_)
		, stateOrSwitchId(stateOrSwitchId_)
		, rtpcValue(rtpcValue_)
	{}

	SSwitchState(SSwitchState const&) = delete;
	SSwitchState(SSwitchState&&) = delete;
	SSwitchState& operator=(SSwitchState const&) = delete;
	SSwitchState& operator=(SSwitchState&&) = delete;

	ESwitchType const type;
	AkUInt32 const    stateOrSwitchGroupId;
	AkUInt32 const    stateOrSwitchId;
	float const       rtpcValue;
};

enum class EEnvironmentType : EnumFlagsType
{
	None,
	AuxBus,
	Rtpc,
};

struct SEnvironment final : public IEnvironment
{
	explicit SEnvironment(EEnvironmentType const type_, AkAuxBusID const busId_)
		: type(type_)
		, busId(busId_)
	{
		CRY_ASSERT(type_ == EEnvironmentType::AuxBus);
	}

	explicit SEnvironment(
		EEnvironmentType const type_,
		AkRtpcID const rtpcId_,
		float const multiplier_,
		float const shift_)
		: type(type_)
		, rtpcId(rtpcId_)
		, multiplier(multiplier_)
		, shift(shift_)
	{
		CRY_ASSERT(type_ == EEnvironmentType::Rtpc);
	}

	SEnvironment(SEnvironment const&) = delete;
	SEnvironment(SEnvironment&&) = delete;
	SEnvironment& operator=(SEnvironment const&) = delete;
	SEnvironment& operator=(SEnvironment&&) = delete;

	EEnvironmentType const type;

	union
	{
		// Aux Bus implementation
		struct
		{
			AkAuxBusID const busId;
		};

		// Rtpc implementation
		struct
		{
			AkRtpcID const rtpcId;
			float const    multiplier;
			float const    shift;
		};
	};
};

class CEvent final : public IEvent, public CPoolObject<CEvent, stl::PSyncNone>
{
public:

	explicit CEvent(CATLEvent& atlEvent_)
		: m_state(EEventState::None)
		, m_id(AK_INVALID_UNIQUE_ID)
		, m_atlEvent(atlEvent_)
		, m_pObject(nullptr)
		, m_maxAttenuation(0.0f)
	{}

	virtual ~CEvent() override;

	CEvent() = delete;
	CEvent(CEvent const&) = delete;
	CEvent(CEvent&&) = delete;
	CEvent& operator=(CEvent const&) = delete;
	CEvent& operator=(CEvent&&) = delete;

	// CryAudio::Impl::IEvent
	virtual ERequestStatus Stop() override;
	// ~CryAudio::Impl::IEvent

	void UpdateVirtualState(float const distance);

	EEventState m_state;
	AkUniqueID  m_id;
	CATLEvent&  m_atlEvent;
	CObject*    m_pObject;
	float       m_maxAttenuation;
};

struct SFile final : public IFile
{
	SFile() = default;

	SFile(SFile const&) = delete;
	SFile(SFile&&) = delete;
	SFile& operator=(SFile const&) = delete;
	SFile& operator=(SFile&&) = delete;

	AkBankID bankId = AK_INVALID_BANK_ID;
};

struct SStandaloneFile final : public IStandaloneFile
{
	SStandaloneFile() = default;
	SStandaloneFile(SStandaloneFile const&) = delete;
	SStandaloneFile(SStandaloneFile&&) = delete;
	SStandaloneFile& operator=(SStandaloneFile const&) = delete;
	SStandaloneFile& operator=(SStandaloneFile&&) = delete;
};

struct SEnvPairCompare
{
	bool operator()(std::pair<AkAuxBusID, float> const& pair1, std::pair<AkAuxBusID, float> const& pair2) const
	{
		return (pair1.second > pair2.second);
	}
};
} // namespace Wwise
} // namespace Impl
} // namespace CryAudio

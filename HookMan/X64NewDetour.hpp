#pragma once
#include "PolyHook.hpp"

constexpr int jmpLen = 16;
constexpr int callLen = 12;
constexpr int hookingLen = callLen/* + jmpLen*/;

namespace PLH
{
	class X64NewDetour :public AbstractDetour
	{
	public:
		friend class VFuncDetour;
		//Credits DarthTon, evolution536
		X64NewDetour();
		X64NewDetour(X64NewDetour&& other) = default; //move
		X64NewDetour& operator=(X64NewDetour&& other) = default;//move assignment
		X64NewDetour(const X64NewDetour& other) = delete; //copy
		X64NewDetour& operator=(const X64NewDetour& other) = delete; //copy assignment
		virtual ~X64NewDetour();

		virtual bool Hook() override;
		virtual HookType GetType() override;

		void AllocateAt(void* p);
		bool CheckLength();
		void FlushCache();

	protected:
		virtual x86_reg GetIpReg() override;
		virtual void FreeTrampoline() override;
		virtual void WriteJMP(const uintptr_t From, const uintptr_t To) override;
		virtual int GetJMPSize() override;

	private:
		void WriteAbsoluteJMP(const uintptr_t Destination, const uintptr_t JMPDestination);
		void WriteCall(const uintptr_t Destination, const uintptr_t CallDestination);

		bool m_UseRelativeJmp{ false };
	};

	PLH::X64NewDetour::X64NewDetour() :AbstractDetour()
	{
	}

	PLH::X64NewDetour::~X64NewDetour()
	{
		if (m_Hooked)
			UnHook();

		if (m_NeedFree)
			FreeTrampoline();
	}

	//
	// impl
	//

	PLH::HookType PLH::X64NewDetour::GetType()
	{
		return PLH::HookType::X64Detour;
	}

	void PLH::X64NewDetour::AllocateAt(void* p)
	{
		m_Trampoline = (uint8_t*)p;
		m_NeedFree = false;
	}

	bool PLH::X64NewDetour::CheckLength()
	{
		m_hkLength = CalculateLength(m_hkSrc, hookingLen); //More stable 16 byte jmp
		m_OriginalLength = m_hkLength; //We modify hkLength in Relocation routine
		if (m_hkLength == 0)
		{
			return false;

			//m_UseRelativeJmp = true;
			//m_hkLength = CalculateLength(m_hkSrc, 6); //Smaller, less safe 6 byte (jmp could be out of bounds)
			//if (m_hkLength == 0)
			//{
			//	PostError(RuntimeError(RuntimeError::Severity::UnRecoverable, "PolyHook X64NewDetour: Function to small to hook"));
			//	return false;
			//}
		}

		return true;
	}

	bool PLH::X64NewDetour::Hook()
	{
		memcpy(m_OriginalCode, m_hkSrc, m_hkLength);
		memcpy(m_Trampoline, m_hkSrc, m_hkLength);
		WriteAbsoluteJMP((uintptr_t)&m_Trampoline[m_hkLength], (uintptr_t)m_hkSrc + m_hkLength);
		m_hkLength += jmpLen; //Size of the above absolute jmp
		RelocateASM(m_Trampoline, &m_hkLength, (uintptr_t)m_hkSrc, (uintptr_t)m_Trampoline);
		//Write the jmp from our trampoline back to the original

		// Build a far jump to the Destination function. (jmps not to address pointed at but to the value in the address)
		MemoryProtect Protector(m_hkSrc, m_hkLength, PAGE_EXECUTE_READWRITE);
		int HookSize = 0;
		if (m_UseRelativeJmp)
		{
			HookSize = 6;
			m_hkSrc[0] = 0xFF;
			m_hkSrc[1] = 0x25;
			//Write 32Bit Displacement from rip
			*(long*)(m_hkSrc + 2) = CalculateRelativeDisplacement<long>((uintptr_t)m_hkSrc, (uintptr_t)&m_Trampoline[m_hkLength + jmpLen], 6);
			*(uintptr_t*)&m_Trampoline[m_hkLength + jmpLen] = (uintptr_t)m_hkDest; //Write the address into memory at [RIP+Displacement]
		}
		else {
			HookSize = hookingLen;
			WriteCall((uintptr_t)m_hkSrc, (uintptr_t)m_hkDest);
			//WriteAbsoluteJMP((uintptr_t)&m_hkSrc[callLen], (uintptr_t)m_hkDest);
			//WriteAbsoluteJMP((uintptr_t)m_hkSrc, (uintptr_t)m_hkDest);
		}
		//Nop Extra bytes from overwritten opcode
		for (uint_fast16_t i = HookSize; i < m_OriginalLength; i++)
			m_hkSrc[i] = 0x90;

		//PostError(RuntimeError(RuntimeError::Severity::Warning, "PolyHook X64NewDetour: Relocation can be out of range"));
		return true;
	}

	void PLH::X64NewDetour::FlushCache()
	{
		FlushInstructionCache(GetCurrentProcess(), m_hkSrc, m_hkLength);
		m_Hooked = true;
	}

	x86_reg PLH::X64NewDetour::GetIpReg()
	{
		return X86_REG_RIP;
	}

	void PLH::X64NewDetour::FreeTrampoline()
	{
	}

	void PLH::X64NewDetour::WriteAbsoluteJMP(const uintptr_t Destination, const uintptr_t JMPDestination)
	{
		/*push rax
		mov rax ...   //Address to original
		xchg qword ptr ss:[rsp], rax
		ret*/
		uint8_t detour[] = { 0x50, 0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x87, 0x04, 0x24, 0xC3 };
		memcpy((uint8_t*)Destination, detour, sizeof(detour));
		*(uintptr_t*)&((uint8_t*)Destination)[3] = JMPDestination;
	}

	void PLH::X64NewDetour::WriteJMP(const uintptr_t From, const uintptr_t To)
	{
		WriteAbsoluteJMP(From, To);
	}

	void PLH::X64NewDetour::WriteCall(const uintptr_t Destination, const uintptr_t CallDestination)
	{
		// with call!
		uint8_t call_detour[] = {
			0x48, 0xb8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
			0xff, 0xd0 };
		*(uintptr_t*)&((uint8_t*)call_detour)[2] = CallDestination;
		memcpy((uint8_t*)Destination, call_detour, sizeof(call_detour));
	}

	int PLH::X64NewDetour::GetJMPSize()
	{
		return 16;
	}
}
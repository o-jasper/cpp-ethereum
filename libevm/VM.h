/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file VM.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <unordered_map>
#include <secp256k1/secp256k1.h>
#include <libethsupport/Exceptions.h>
#include <libethcore/CommonEth.h>
#include <libethcore/Instruction.h>
#include <libethcore/BlockInfo.h>
#include "FeeStructure.h"
#include "ExtVMFace.h"

namespace eth
{

class VMException: public Exception {};
class StepsDone: public VMException {};
class BreakPointHit: public VMException {};
class BadInstruction: public VMException {};
class OutOfGas: public VMException {};
class StackTooSmall: public VMException { public: StackTooSmall(u256 _req, u256 _got): req(_req), got(_got) {} u256 req; u256 got; };
class OperandOutOfRange: public VMException { public: OperandOutOfRange(u256 _min, u256 _max, u256 _got): mn(_min), mx(_max), got(_got) {} u256 mn; u256 mx; u256 got; };

// Convert from a 256-bit integer stack/memory entry into a 160-bit Address hash.
// Currently we just pull out the right (low-order in BE) 160-bits.
inline Address asAddress(u256 _item)
{
	return right160(h256(_item));
}

inline u256 fromAddress(Address _a)
{
	return (u160)_a;
//	h256 ret;
//	memcpy(&ret, &_a, sizeof(_a));
//	return ret;
}

/**
 */
class VM
{
	template <unsigned T> friend class UnitTest;

public:
	/// Construct VM object.
	explicit VM(u256 _gas = 0) { reset(_gas); }

	void reset(u256 _gas = 0);

	template <class Ext>
	bytesConstRef go(Ext& _ext, uint64_t _steps = (uint64_t)-1);

	void require(u256 _n) { if (m_stack.size() < _n) throw StackTooSmall(_n, m_stack.size()); }
	void requireMem(unsigned _n) { if (m_temp.size() < _n) { m_temp.resize(_n); } }
	u256 gas() const { return m_gas; }
	u256 curPC() const { return m_curPC; }

	bytes const& memory() const { return m_temp; }
	u256s const& stack() const { return m_stack; }

private:
	u256 m_gas = 0;
	u256 m_curPC = 0;
	bytes m_temp;
	u256s m_stack;
};

}

// INLINE:
template <class Ext> eth::bytesConstRef eth::VM::go(Ext& _ext, uint64_t _steps)
{
	u256 nextPC = m_curPC + 1;
	for (bool stopped = false; !stopped && _steps--; m_curPC = nextPC, nextPC = m_curPC + 1)
	{
		// INSTRUCTION...
		Instruction inst = (Instruction)_ext.getCode(m_curPC);

		// FEES...
		bigint runGas = c_stepGas;
		unsigned newTempSize = (unsigned)m_temp.size();
		switch (inst)
		{
		case Instruction::STOP:
			runGas = 0;
			break;

		case Instruction::SUICIDE:
			runGas = 0;
			break;

		case Instruction::SSTORE:
			require(2);
			if (!_ext.store(m_stack.back()) && m_stack[m_stack.size() - 2])
				runGas = c_sstoreGas * 2;
			else if (_ext.store(m_stack.back()) && !m_stack[m_stack.size() - 2])
				runGas = 0;
			else
				runGas = c_sstoreGas;
			break;

		case Instruction::SLOAD:
			runGas += c_sloadGas;
			break;

		// These all operate on memory and therefore potentially expand it:
		case Instruction::MSTORE:
			require(2);
			newTempSize = (unsigned)m_stack.back() + 32;
			break;
		case Instruction::MSTORE8:
			require(2);
			newTempSize = (unsigned)m_stack.back() + 1;
			break;
		case Instruction::MLOAD:
			require(1);
			newTempSize = (unsigned)m_stack.back() + 32;
			break;
		case Instruction::RETURN:
			require(2);
			newTempSize = (unsigned)m_stack.back() + (unsigned)m_stack[m_stack.size() - 2];
			break;
		case Instruction::SHA3:
			require(2);
			runGas = c_sha3Gas;
			newTempSize = (unsigned)m_stack.back() + (unsigned)m_stack[m_stack.size() - 2];
			break;
		case Instruction::ECRECOVER:
			require(3);
			runGas = c_ecrecoverGas;
			newTempSize = (unsigned)m_stack.back() + (unsigned)m_stack[m_stack.size() - 3];
			break;
		case Instruction::CALLDATACOPY:
			require(3);
			newTempSize = (unsigned)m_stack.back() + (unsigned)m_stack[m_stack.size() - 3];
			break;
		case Instruction::CODECOPY:
			require(3);
			newTempSize = (unsigned)m_stack.back() + (unsigned)m_stack[m_stack.size() - 3];
			break;

		case Instruction::BALANCE:
			runGas = c_balanceGas;
			break;

		case Instruction::CALL:
			require(7);
			runGas = c_callGas + (unsigned)m_stack[m_stack.size() - 1];
			newTempSize = std::max((unsigned)m_stack[m_stack.size() - 6] + (unsigned)m_stack[m_stack.size() - 7], (unsigned)m_stack[m_stack.size() - 4] + (unsigned)m_stack[m_stack.size() - 5]);
			break;

		case Instruction::CREATE:
		{
			require(3);
			unsigned inOff = (unsigned)m_stack[m_stack.size() - 2];
			unsigned inSize = (unsigned)m_stack[m_stack.size() - 3];
			newTempSize = inOff + inSize;
			runGas += c_createGas;
			break;
		}

		default:
			break;
		}

		newTempSize = (newTempSize + 31) / 32 * 32;
		if (newTempSize > m_temp.size())
			runGas += c_memoryGas * (newTempSize - m_temp.size()) / 32;

		if (m_gas < runGas)
		{
			// Out of gas!
			m_gas = 0;
			throw OutOfGas();
		}

		m_gas = (u256)((bigint)m_gas - runGas);

		if (newTempSize > m_temp.size())
			m_temp.resize(newTempSize);

		// EXECUTE...
		switch (inst)
		{
		case Instruction::ADD:
			//pops two items and pushes S[-1] + S[-2] mod 2^256.
			require(2);
			m_stack[m_stack.size() - 2] += m_stack.back();
			m_stack.pop_back();
			break;
		case Instruction::MUL:
			//pops two items and pushes S[-1] * S[-2] mod 2^256.
			require(2);
			m_stack[m_stack.size() - 2] *= m_stack.back();
			m_stack.pop_back();
			break;
		case Instruction::SUB:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack.back() - m_stack[m_stack.size() - 2];
			m_stack.pop_back();
			break;
		case Instruction::DIV:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack[m_stack.size() - 2] ? m_stack.back() / m_stack[m_stack.size() - 2] : 0;
			m_stack.pop_back();
			break;
		case Instruction::SDIV:
			require(2);
			(s256&)m_stack[m_stack.size() - 2] = m_stack[m_stack.size() - 2] ? (s256&)m_stack.back() / (s256&)m_stack[m_stack.size() - 2] : 0;
			m_stack.pop_back();
			break;
		case Instruction::MOD:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack[m_stack.size() - 2] ? m_stack.back() % m_stack[m_stack.size() - 2] : 0;
			m_stack.pop_back();
			break;
		case Instruction::SMOD:
			require(2);
			(s256&)m_stack[m_stack.size() - 2] = m_stack[m_stack.size() - 2] ? (s256&)m_stack.back() % (s256&)m_stack[m_stack.size() - 2] : 0;
			m_stack.pop_back();
			break;
		case Instruction::EXP:
		{
			require(2);
			auto base = m_stack.back();
			unsigned expon = (unsigned)m_stack[m_stack.size() - 2];
			m_stack.pop_back();
			m_stack.back() = boost::multiprecision::pow(base, expon);
			break;
		}
		case Instruction::NEG:
			require(1);
			m_stack.back() = ~(m_stack.back() - 1);
			break;
		case Instruction::LT:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack.back() < m_stack[m_stack.size() - 2] ? 1 : 0;
			m_stack.pop_back();
			break;
		case Instruction::GT:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack.back() > m_stack[m_stack.size() - 2] ? 1 : 0;
			m_stack.pop_back();
			break;
		case Instruction::SLT:
			require(2);
			m_stack[m_stack.size() - 2] = (s256&)m_stack.back() < (s256&)m_stack[m_stack.size() - 2] ? 1 : 0;
			m_stack.pop_back();
			break;
		case Instruction::SGT:
			require(2);
			m_stack[m_stack.size() - 2] = (s256&)m_stack.back() > (s256&)m_stack[m_stack.size() - 2] ? 1 : 0;
			m_stack.pop_back();
			break;
		case Instruction::EQ:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack.back() == m_stack[m_stack.size() - 2] ? 1 : 0;
			m_stack.pop_back();
			break;
		case Instruction::NOT:
			require(1);
			m_stack.back() = m_stack.back() ? 0 : 1;
			break;
		case Instruction::AND:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack.back() & m_stack[m_stack.size() - 2];
			m_stack.pop_back();
			break;
		case Instruction::OR:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack.back() | m_stack[m_stack.size() - 2];
			m_stack.pop_back();
			break;
		case Instruction::XOR:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack.back() ^ m_stack[m_stack.size() - 2];
			m_stack.pop_back();
			break;
		case Instruction::BYTE:
			require(2);
			m_stack[m_stack.size() - 2] = m_stack.back() < 32 ? (m_stack[m_stack.size() - 2] >> (uint)(8 * (31 - m_stack.back()))) & 0xff : 0;
			m_stack.pop_back();
			break;
		case Instruction::SHA3:
		{
			require(2);
			unsigned inOff = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned inSize = (unsigned)m_stack.back();
			m_stack.pop_back();
			m_stack.push_back(sha3(bytesConstRef(m_temp.data() + inOff, inSize)));
			break;
		}
		case Instruction::ECRECOVER:
		{
			// msghash, r, s should be at top of stack
			// puts recoverd pubkey on stack, or 0 if recover fails
			require(3);
			h256 msgHash = m_stack.back();
			m_stack.pop_back();
			h256 sig[2];
			sig[0] = m_stack.back();
			m_stack.pop_back();
			sig[1] = m_stack.back();
			m_stack.pop_back();

			byte pubkey[65];
			h160 address;	
			int pubkeyLength = 65;
			if (secp256k1_ecdsa_recover_compact(msgHash.data(), 32, sig[0].data(), pubkey, &pubkeyLength, 0, (byte)1)){

				address = right160(eth::sha3(bytesConstRef(&(pubkey[1]), 64)));
				m_stack.push_back(fromAddress(address));
			}
			else
				m_stack.push_back((u256)0);
			break;
		}
		case Instruction::ADDRESS:
			m_stack.push_back(fromAddress(_ext.myAddress));
			break;
		case Instruction::ORIGIN:
			m_stack.push_back(fromAddress(_ext.origin));
			break;
		case Instruction::BALANCE:
		{
			require(1);
			m_stack.back() = _ext.balance(asAddress(m_stack.back()));
			break;
		}
		case Instruction::CALLER:
			m_stack.push_back(fromAddress(_ext.caller));
			break;
		case Instruction::CALLVALUE:
			m_stack.push_back(_ext.value);
			break;
		case Instruction::CALLDATALOAD:
		{
			require(1);
			if ((unsigned)m_stack.back() + 31 < _ext.data.size())
				m_stack.back() = (u256)*(h256 const*)(_ext.data.data() + (unsigned)m_stack.back());
			else
			{
				h256 r;
				for (unsigned i = (unsigned)m_stack.back(), e = (unsigned)m_stack.back() + 32, j = 0; i < e; ++i, ++j)
					r[j] = i < _ext.data.size() ? _ext.data[i] : 0;
				m_stack.back() = (u256)r;
			}
			break;
		}
		case Instruction::CALLDATASIZE:
			m_stack.push_back(_ext.data.size());
			break;
		case Instruction::CALLDATACOPY:
		{
			require(3);
			unsigned mf = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned cf = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned l = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned el = cf + l > _ext.data.size() ? _ext.data.size() < cf ? 0 : _ext.data.size() - cf : l;
			memcpy(m_temp.data() + mf, _ext.data.data() + cf, el);
			memset(m_temp.data() + mf + el, 0, l - el);
			break;
		}
		case Instruction::CODESIZE:
			m_stack.push_back(_ext.code.size());
			break;
		case Instruction::CODECOPY:
		{
			require(3);
			unsigned mf = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned cf = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned l = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned el = cf + l > _ext.code.size() ? _ext.code.size() < cf ? 0 : _ext.code.size() - cf : l;
			memcpy(m_temp.data() + mf, _ext.code.data() + cf, el);
			memset(m_temp.data() + mf + el, 0, l - el);
			break;
		}
		case Instruction::GASPRICE:
			m_stack.push_back(_ext.gasPrice);
			break;
		case Instruction::PREVHASH:
			m_stack.push_back(_ext.previousBlock.hash);
			break;
		case Instruction::COINBASE:
			m_stack.push_back((u160)_ext.currentBlock.coinbaseAddress);
			break;
		case Instruction::TIMESTAMP:
			m_stack.push_back(_ext.currentBlock.timestamp);
			break;
		case Instruction::NUMBER:
			m_stack.push_back(_ext.currentBlock.number);
			break;
		case Instruction::DIFFICULTY:
			m_stack.push_back(_ext.currentBlock.difficulty);
			break;
		case Instruction::GASLIMIT:
			m_stack.push_back(1000000);
			break;
		case Instruction::PUSH1:
		case Instruction::PUSH2:
		case Instruction::PUSH3:
		case Instruction::PUSH4:
		case Instruction::PUSH5:
		case Instruction::PUSH6:
		case Instruction::PUSH7:
		case Instruction::PUSH8:
		case Instruction::PUSH9:
		case Instruction::PUSH10:
		case Instruction::PUSH11:
		case Instruction::PUSH12:
		case Instruction::PUSH13:
		case Instruction::PUSH14:
		case Instruction::PUSH15:
		case Instruction::PUSH16:
		case Instruction::PUSH17:
		case Instruction::PUSH18:
		case Instruction::PUSH19:
		case Instruction::PUSH20:
		case Instruction::PUSH21:
		case Instruction::PUSH22:
		case Instruction::PUSH23:
		case Instruction::PUSH24:
		case Instruction::PUSH25:
		case Instruction::PUSH26:
		case Instruction::PUSH27:
		case Instruction::PUSH28:
		case Instruction::PUSH29:
		case Instruction::PUSH30:
		case Instruction::PUSH31:
		case Instruction::PUSH32:
		{
			int i = (int)inst - (int)Instruction::PUSH1 + 1;
			nextPC = m_curPC + 1;
			m_stack.push_back(0);
			for (; i--; nextPC++)
				m_stack.back() = (m_stack.back() << 8) | _ext.getCode(nextPC);
			break;
		}
		case Instruction::POP:
			require(1);
			m_stack.pop_back();
			break;
		case Instruction::DUP:
			require(1);
			m_stack.push_back(m_stack.back());
			break;
		/*case Instruction::DUPN:
		{
			auto s = store(curPC + 1);
			if (s == 0 || s > stack.size())
				throw OperandOutOfRange(1, stack.size(), s);
			stack.push_back(stack[stack.size() - (uint)s]);
			nextPC = curPC + 2;
			break;
		}*/
		case Instruction::SWAP:
		{
			require(2);
			auto d = m_stack.back();
			m_stack.back() = m_stack[m_stack.size() - 2];
			m_stack[m_stack.size() - 2] = d;
			break;
		}
		/*case Instruction::SWAPN:
		{
			require(1);
			auto d = stack.back();
			auto s = store(curPC + 1);
			if (s == 0 || s > stack.size())
				throw OperandOutOfRange(1, stack.size(), s);
			stack.back() = stack[stack.size() - (uint)s];
			stack[stack.size() - (uint)s] = d;
			nextPC = curPC + 2;
			break;
		}*/
		case Instruction::MLOAD:
		{
			require(1);
			m_stack.back() = (u256)*(h256 const*)(m_temp.data() + (unsigned)m_stack.back());
			break;
		}
		case Instruction::MSTORE:
		{
			require(2);
			*(h256*)&m_temp[(unsigned)m_stack.back()] = (h256)m_stack[m_stack.size() - 2];
			m_stack.pop_back();
			m_stack.pop_back();
			break;
		}
		case Instruction::MSTORE8:
		{
			require(2);
			m_temp[(unsigned)m_stack.back()] = (byte)(m_stack[m_stack.size() - 2] & 0xff);
			m_stack.pop_back();
			m_stack.pop_back();
			break;
		}
		case Instruction::SLOAD:
			require(1);
			m_stack.back() = _ext.store(m_stack.back());
			break;
		case Instruction::SSTORE:
			require(2);
			_ext.setStore(m_stack.back(), m_stack[m_stack.size() - 2]);
			m_stack.pop_back();
			m_stack.pop_back();
			break;
		case Instruction::JUMP:
			require(1);
			nextPC = m_stack.back();
			m_stack.pop_back();
			break;
		case Instruction::JUMPI:
			require(2);
			if (m_stack[m_stack.size() - 2])
				nextPC = m_stack.back();
			m_stack.pop_back();
			m_stack.pop_back();
			break;
		case Instruction::PC:
			m_stack.push_back(m_curPC);
			break;
		case Instruction::MEMSIZE:
			m_stack.push_back(m_temp.size());
			break;
		case Instruction::GAS:
			m_stack.push_back(m_gas);
			break;
		case Instruction::CREATE:
		{
			require(3);

			u256 endowment = m_stack.back();
			m_stack.pop_back();
			unsigned initOff = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned initSize = (unsigned)m_stack.back();
			m_stack.pop_back();

			if (_ext.balance(_ext.myAddress) >= endowment)
			{
				_ext.subBalance(endowment);
				m_stack.push_back((u160)_ext.create(endowment, &m_gas, bytesConstRef(m_temp.data() + initOff, initSize)));
			}
			else
				m_stack.push_back(0);
			break;
		}
		case Instruction::CALL:
		{
			require(7);

			u256 gas = m_stack.back();
			m_stack.pop_back();
			u160 receiveAddress = asAddress(m_stack.back());
			m_stack.pop_back();
			u256 value = m_stack.back();
			m_stack.pop_back();

			unsigned inOff = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned inSize = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned outOff = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned outSize = (unsigned)m_stack.back();
			m_stack.pop_back();

			if (_ext.balance(_ext.myAddress) >= value)
			{
				_ext.subBalance(value);
				m_stack.push_back(_ext.call(receiveAddress, value, bytesConstRef(m_temp.data() + inOff, inSize), &gas, bytesRef(m_temp.data() + outOff, outSize)));
			}
			else
				m_stack.push_back(0);

			m_gas += gas;
			break;
		}
		case Instruction::RETURN:
		{
			require(2);

			unsigned b = (unsigned)m_stack.back();
			m_stack.pop_back();
			unsigned s = (unsigned)m_stack.back();
			m_stack.pop_back();

			return bytesConstRef(m_temp.data() + b, s);
		}
		case Instruction::SUICIDE:
		{
			require(1);
			Address dest = asAddress(m_stack.back());
			_ext.suicide(dest);
			// ...follow through to...
		}
		case Instruction::STOP:
			return bytesConstRef();
		default:
			throw BadInstruction();
		}
	}
	if (_steps == (uint64_t)-1)
		throw StepsDone();
	return bytesConstRef();
}


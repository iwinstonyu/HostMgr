//
//	<one line to give the program's name and a brief idea of what it does.>
//	Copyright (C) 2017. WenJin Yu. windpenguin@gmail.com.
//
//	Created at 2017/10/10 15:01:43
//	Version 1.0
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <memory>

namespace wind {

// 单生产者-单消费者 fifo 队列
// readCount_和writeCount_循环uint32使用，在maxSize_合理的情况下不会有问题

template<typename T>
class SingleQueue {
public:
	typedef std::shared_ptr<T> TRef;
	struct TNode;
	typedef std::shared_ptr<TNode> TNodeRef;
	struct TNode {
		TNode() : hasNext_(false) {}
		TNode(TRef dataRef) : dataRef_(dataRef), hasNext_(false) {}

		TRef dataRef_;
		volatile bool hasNext_;
		TNodeRef next_;
	};

public:
	explicit SingleQueue(unsigned int maxSize=50000) : maxSize_(maxSize), writeCount_(0), readCount_(0) {
		write_.reset(new TNode());
		read_ = write_;
	}

	~SingleQueue() {}

	unsigned int Size() { return (writeCount_ - readCount_); }

	bool Write(TRef dataRef) {
		if (Size() > maxSize_)
			return false;

		TNodeRef nodeRef(new TNode(dataRef));
		if (!nodeRef)
			return false;

		write_->next_ = nodeRef;
		++writeCount_;
		MemoryBarrier();
		write_->hasNext_ = true;
		write_ = nodeRef;

		return true;
	}

	TRef Read() {
		if (!read_->hasNext_)
			return TRef();

		++readCount_;
		read_ = read_->next_;
		return read_->dataRef_;
	}

private:
	SingleQueue(const SingleQueue<T>&);
	SingleQueue<T>& operator=(const SingleQueue<T>&);

	TNodeRef write_;
	TNodeRef read_;
	unsigned int writeCount_;
	unsigned int readCount_;
	const unsigned int maxSize_;
};

} // namespace wind
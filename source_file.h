#ifndef SOURCE_FILE_H_
#define SOURCE_FILE_H_

#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <exception>
#include "annotation_item.h"

class SourceFile
{
public:
	SourceFile();
	~SourceFile();
	void * Malloc(int64_t size);
	void Free();
	void ReadyToMove();
	void JumpTo(int64_t location);
	int64_t MoveNext();
	// content
	char * content_;
	int64_t content_size_;
	int64_t index_;
	// line
	std::vector<int64_t> line_table_;
	int64_t line_size_;
	int64_t line_index_;
	int64_t line_;
	// annotation
	std::vector<AnnotationItem *> annotation_table_;
	int64_t annotation_size_;
	int64_t annotation_index_;
	bool annotation_;
private:
	// lock of moving
	bool move_enabled_;
};

SourceFile::SourceFile()
{
	content_ = NULL;
	content_size_ = 0;
	index_ = -1;
	line_size_ = 0;
	line_index_ = -1;
	line_ = -1;
	annotation_size_ = -1;
	annotation_index_ = -2;
	annotation_ = false;
	move_enabled_ = false;
}

SourceFile::~SourceFile()
{
	Free();
	for (int64_t i = 0; i < annotation_table_.size(); ++i)
	{
		AnnotationItem::s_Free(annotation_table_[i]);
		annotation_table_[i] = NULL;
	}
}

void * SourceFile::Malloc(int64_t size)
{
	if (size <= 0)
	{
		throw std::exception("Function \"void * SourceFile::Malloc(int64_t size)\" says: Invalid parameter \"size\".");
	}
	Free();
	content_ = new char[size];
	return content_;
}

void SourceFile::Free()
{
	if (NULL != content_)
	{
		delete[] content_;
		content_ = NULL;
	}
}

void SourceFile::ReadyToMove()
{
	// unlock
	move_enabled_ = true;
	// jump to the beginning
	JumpTo(0);
}

void SourceFile::JumpTo(int64_t location)
{
	if (false == move_enabled_)
	{
		throw std::exception("Function \"void * SourceFile::Malloc(int64_t size)\" says: You should call function \"void SourceFile::ReadyToMove()\" first.");
	}
	if (location < 0 || location > content_size_ - 1)
	{
		throw std::exception("Function \"void * SourceFile::Malloc(int64_t size)\" says: Invalid parameter \"size\".");
	}
	// content index
	index_ = location;
	// line
	line_index_ = 0;
	for (int64_t i = 0; i < line_size_; ++i)
	{
		if (location >= line_table_[line_index_])
		{
			line_index_ += 1;
		}
		else
		{
			break;
		}
	}
	line_index_ -= 1;
	line_ = line_table_[line_index_];
	// annotation
	if (0 == annotation_size_)
	{
		// No annotation exists. Function "MoveNext" will do nothing about annotation.
		annotation_index_ = -2;
		annotation_ = false;
	}
	else
	{
		annotation_index_ = 0;
		for (int64_t i = 0; i < annotation_size_; ++i)
		{
			if (location >= annotation_table_[annotation_index_]->beginning_)
			{
				annotation_index_ += 1;
			}
			else
			{
				break;
			}
		}
		annotation_index_ -= 1;
		if (annotation_index_ >= 0)
		{
			// There is an annotation at the beginning of the source file.
			if (location <= annotation_table_[annotation_index_]->end_)
			{
				annotation_ = true;
			}
			else
			{
				annotation_ = false;
			}
		}
		else
		{
			annotation_ = false;
		}
	}
}

int64_t SourceFile::MoveNext()
{
	if (false == move_enabled_)
	{
		throw std::exception("Function \"int64_t SourceFile::MoveNext()\" says: You should call function \"void SourceFile::ReadyToMove()\" first.");
	}
	if (index_ == content_size_ - 1)
	{
		return 0;
	}
	// move
	index_ += 1;
	// line
	if (line_index_ + 1 < line_size_ && index_ >= line_table_[line_index_ + 1])
	{
		line_index_ += 1;
		line_ = line_table_[line_index_];
	}
	// annotation
	if (-2 != annotation_index_)
	{
		if (annotation_)
		{
			if (index_ > annotation_table_[annotation_index_]->end_)
			{
				// annotation => non-annotation
				annotation_ = false;
			}
		}
		else
		{
			if (annotation_index_ + 1 < annotation_size_ && index_ >= annotation_table_[annotation_index_ + 1]->beginning_)
			{
				// non-annotation => annotation
				annotation_index_ += 1;
				annotation_ = true;
			}
		}
	}
	return 1;
}

#endif

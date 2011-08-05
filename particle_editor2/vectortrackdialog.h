// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef VECTOR_TRACK_DIALOG_H
#define VECTOR_TRACK_DIALOG_H

namespace frozenbyte
{
namespace particle
{

struct VectorTrackDialogData;
class VectorTrackDialog {
	boost::scoped_ptr<VectorTrackDialogData> data;
public:
	VectorTrackDialog();
	~VectorTrackDialog();
	void open(editor::Dialog& parent, int resourceID, editor::ParserGroup& parser, bool floatMode=false);
//	const editor::ParserGroup& getParserGroup();
};

} // particle
} // frozenbyte

#endif
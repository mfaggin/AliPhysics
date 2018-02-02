///
/// \file AliFemtoCorrFctn.h
///

#pragma once

#ifndef ALIFEMTOCORRFCTN_H
#define ALIFEMTOCORRFCTN_H

#include "AliFemtoAnalysis.h"
#include "AliFemtoEvent.h"
#include "AliFemtoPair.h"
#include "AliFemtoPairCut.h"


/// \class AliFemtoCorrFctn
/// \brief The pure-virtual base class for correlation functions
///
/// All correlation function classes must inherit from this one.
///
/// This class has a optional pointers to the "parent" analysis and
/// a pair cut
///
class AliFemtoCorrFctn {

  friend class AliFemtoAnalysis;

public:

  /// Construct with null pointers to "parent" analysis and pair-cut
  AliFemtoCorrFctn();
  /// Copy constructor - copy pointer to parent analysis, clones pair-cut if it exists
  AliFemtoCorrFctn(const AliFemtoCorrFctn&);
  virtual ~AliFemtoCorrFctn(){/* no-op */};
  /// Assignment operator - shallow copy of analysis and cut
  AliFemtoCorrFctn& operator=(const AliFemtoCorrFctn&);

  virtual AliFemtoString Report() = 0;

  /// Not Implemented - Add signal pair
  virtual void AddRealPair(AliFemtoPair* aPair);
  /// Not Implemented - Add background pair
  virtual void AddMixedPair(AliFemtoPair* aPir);

  /// Not Implemented - Add pair with optional
  virtual void AddFirstParticle(AliFemtoParticle *particle, bool mixing);
  virtual void AddSecondParticle(AliFemtoParticle *particle);
  virtual void CalculateAnglesForEvent();

  virtual void EventBegin(const AliFemtoEvent* aEvent);
  virtual void EventEnd(const AliFemtoEvent* aEvent);
  virtual void Finish() = 0;

  virtual TList* GetOutputList() = 0;

  virtual AliFemtoCorrFctn* Clone() const = 0;

  AliFemtoAnalysis* HbtAnalysis(){return fyAnalysis;};
  void SetAnalysis(AliFemtoAnalysis* aAnalysis);
  void SetPairSelectionCut(AliFemtoPairCut* aCut);

protected:
  AliFemtoAnalysis* fyAnalysis; //! link to the analysis
  AliFemtoPairCut* fPairCut;    //! this is a PairSelection criteria for this Correlation Function

  private:

#ifdef __ROOT__
  /// \cond CLASSIMP
  ClassDef(AliFemtoCorrFctn, 1);
  /// \endcond
#endif
};

inline void AliFemtoCorrFctn::SetAnalysis(AliFemtoAnalysis* analysis)
{
  fyAnalysis = analysis;
}

inline void AliFemtoCorrFctn::SetPairSelectionCut(AliFemtoPairCut* cut)
{
  fPairCut = cut;
}

inline void AliFemtoCorrFctn::EventBegin(const AliFemtoEvent* /* event */)
{ // no-op
}

inline void AliFemtoCorrFctn::EventEnd(const AliFemtoEvent* /* event */)
{  // no-op
}




#endif  // ALIFEMTOCORRFCTN_H

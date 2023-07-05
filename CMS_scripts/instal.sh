cmsrel CMSSW_12_1_0
cd CMSSW_12_1_0/src
cmsenv
git cms-init
git cms-addpkg Geometry/CMSCommonData
git cms-addpkg Configuration/Geometry
git cms-addpkg PhysicsTools/PatAlgos
git cms-addpkg RecoTracker

## Modif 1:

line_num_a=58
line_text_a="        MaxNumberOfPixelClusters = 1000000,"
file_path_a=RecoTracker/ConversionSeedGenerators/python/PhotonConversionTrajectorySeedProducerFromSingleLeg_cfi.py

sed -i "${line_num_a}s/.*/${line_text_a}/" ${file_path_a}

## Modif 2:

file_path_b=PhysicsTools/PatAlgos/python/slimming/slimming_cff.py
file_text_b=$(cat <<-END >> ${file_path_b}
import FWCore.ParameterSet.Config as cms
from PhysicsTools.PatAlgos.slimming.packedPFCandidates_cff import *
from PhysicsTools.PatAlgos.slimming.isolatedTracks_cfi import *
from PhysicsTools.PatAlgos.slimming.lostTracks_cfi import *
from PhysicsTools.PatAlgos.slimming.offlineSlimmedPrimaryVertices_cfi import *
from PhysicsTools.PatAlgos.slimming.offlineSlimmedPrimaryVertices4D_cfi import *
from PhysicsTools.PatAlgos.slimming.offlineSlimmedPrimaryVerticesWithBS_cfi import *
from CommonTools.RecoAlgos.primaryVertexAssociation_cfi import *
from PhysicsTools.PatAlgos.slimming.genParticles_cff import *
from PhysicsTools.PatAlgos.slimming.genParticleAssociation_cff import *
from PhysicsTools.PatAlgos.slimming.selectedPatTrigger_cfi import *
from PhysicsTools.PatAlgos.slimming.slimmedPatTrigger_cfi import *
from PhysicsTools.PatAlgos.slimming.slimmedJets_cfi      import *
from PhysicsTools.PatAlgos.slimming.slimmedCaloJets_cfi  import *
from PhysicsTools.PatAlgos.slimming.slimmedGenJets_cfi   import *
from PhysicsTools.PatAlgos.slimming.slimmedElectrons_cfi import *
from PhysicsTools.PatAlgos.slimming.slimmedLowPtElectrons_cff import *
from PhysicsTools.PatAlgos.slimming.slimmedTrackExtras_cff import *
from PhysicsTools.PatAlgos.slimming.slimmedMuons_cfi     import *
from PhysicsTools.PatAlgos.slimming.slimmedPhotons_cfi   import *
from PhysicsTools.PatAlgos.slimming.slimmedOOTPhotons_cff import *
from PhysicsTools.PatAlgos.slimming.slimmedTaus_cfi      import *
from PhysicsTools.PatAlgos.slimming.slimmedSecondaryVertices_cfi      import *
from PhysicsTools.PatAlgos.slimming.slimmedMETs_cfi      import *
from PhysicsTools.PatAlgos.slimming.slimmedV0s_cff      import *
from PhysicsTools.PatAlgos.slimming.metFilterPaths_cff   import *
from PhysicsTools.PatAlgos.slimming.MicroEventContent_cff import *
from RecoEgamma.EgammaPhotonProducers.reducedEgamma_cfi  import *
from RecoLuminosity.LumiProducer.bunchSpacingProducer_cfi import bunchSpacingProducer
from HeavyFlavorAnalysis.Onia2MuMu.OniaPhotonConversionProducer_cfi import PhotonCandidates as oniaPhotonCandidates
from RecoLocalCalo.HcalRecProducers.HcalHitSelection_cfi import *
from PhysicsTools.PatAlgos.packedCandidateMuonID_cfi import packedCandidateMuonID
from PhysicsTools.PatAlgos.packedPFCandidateTrackChi2_cfi import packedPFCandidateTrackChi2
from RecoHI.HiCentralityAlgos.CentralityBin_cfi import centralityBin
from RecoHI.HiCentralityAlgos.hiHFfilters_cfi import hiHFfilters
lostTrackChi2 = packedPFCandidateTrackChi2.clone(candidates = "lostTracks", doLostTracks = True)
slimmingTask = cms.Task(
    packedPFCandidatesTask,
    lostTracks,
    isolatedTracks,
    offlineSlimmedPrimaryVertices,
    offlineSlimmedPrimaryVerticesWithBS,
    primaryVertexAssociation,
    primaryVertexWithBSAssociation,
    genParticlesTask,
    packedCandidateToGenAssociationTask,
    selectedPatTrigger,
    slimmedPatTrigger,
    slimmedCaloJets,
    slimmedJets,
    slimmedJetsAK8,
    slimmedGenJets,
    slimmedGenJetsAK8,
    slimmedElectrons,
    slimmedLowPtElectronsTask,
    slimmedMuonTrackExtras,
    slimmedMuons,
    slimmedPhotons,
    #slimmedOOTPhotons,
    slimmedTaus,
    slimmedSecondaryVertices,
    slimmedKshortVertices,
    slimmedLambdaVertices,
    slimmedMETs,
    metFilterPathsTask,
    reducedEgamma,
    slimmedHcalRecHits,
    bunchSpacingProducer,
    oniaPhotonCandidates,
    packedCandidateMuonID, # added by hand
    packedPFCandidateTrackChi2, # added by hand
    lostTrackChi2,
    centralityBin,
    hiHFfilters
)
from Configuration.ProcessModifiers.pp_on_AA_cff import pp_on_AA
pp_on_AA.toReplaceWith(slimmingTask, slimmingTask.copyAndExclude([slimmedOOTPhotons]))
from Configuration.Eras.Modifier_run2_miniAOD_94XFall17_cff import run2_miniAOD_94XFall17
from Configuration.Eras.Modifier_run2_miniAOD_80XLegacy_cff import run2_miniAOD_80XLegacy
_mAOD = (run2_miniAOD_94XFall17 | run2_miniAOD_80XLegacy)
(pp_on_AA | _mAOD).toReplaceWith(slimmingTask,
                                 slimmingTask.copyAndExclude([slimmedLowPtElectronsTask]))
from PhysicsTools.PatAlgos.slimming.hiPixelTracks_cfi import hiPixelTracks
from RecoHI.HiEvtPlaneAlgos.HiEvtPlane_cfi import hiEvtPlane
from RecoHI.HiEvtPlaneAlgos.hiEvtPlaneFlat_cfi import hiEvtPlaneFlat
pp_on_AA.toReplaceWith(slimmingTask, cms.Task(slimmingTask.copy(), hiPixelTracks, hiEvtPlane, hiEvtPlaneFlat))

#from RecoHI.HiCentralityAlgos.CentralityBin_cfi import centralityBin
#from RecoHI.HiCentralityAlgos.hiHFfilters_cfi import hiHFfilters
#lostTrackChi2 = packedPFCandidateTrackChi2.clone(candidates = "lostTracks", doLostTracks = True)
pp_on_AA.toReplaceWith(
    slimmingTask,
    cms.Task(slimmingTask.copy(), packedCandidateMuonID, packedPFCandidateTrackChi2, lostTrackChi2, centralityBin, hiHFfilters))
from Configuration.ProcessModifiers.run2_miniAOD_pp_on_AA_103X_cff import run2_miniAOD_pp_on_AA_103X
run2_miniAOD_pp_on_AA_103X.toReplaceWith(slimmingTask,cms.Task(primaryVertexAssociationCleaned,slimmingTask.copy()))
run2_miniAOD_pp_on_AA_103X.toReplaceWith(slimmingTask,cms.Task(primaryVertexWithBSAssociationCleaned,slimmingTask.copy()))
from RecoHI.HiTracking.miniAODVertexRecovery_cff import offlinePrimaryVerticesRecovery, offlineSlimmedPrimaryVerticesRecovery
pp_on_AA.toReplaceWith(
    slimmingTask,
    cms.Task(slimmingTask.copy(), offlinePrimaryVerticesRecovery, offlineSlimmedPrimaryVerticesRecovery))
from Configuration.Eras.Modifier_phase2_timing_cff import phase2_timing
_phase2_timing_slimmingTask = cms.Task(slimmingTask.copy(),
                                       offlineSlimmedPrimaryVertices4D)
phase2_timing.toReplaceWith(slimmingTask,_phase2_timing_slimmingTask)
END

)

## modif 3:

directory_path_c=Configuration/GenProduction/python/
file_path_c=${directory_path_c}JpsiMuMu_5p5TeV_Pythia8TuneCP5_cfi.py
mkdir -p ${directory_path_c}
file_text_b=$(cat <<-END >> ${file_path_c}
import FWCore.ParameterSet.Config as cms
from Configuration.Generator.Pythia8CommonSettings_cfi import *
from Configuration.Generator.MCTunes2017.PythiaCP5Settings_cfi import *

generator = cms.EDFilter("Pythia8GeneratorFilter",
                         pythiaPylistVerbosity = cms.untracked.int32(0),
                         filterEfficiency = cms.untracked.double(0),
                         pythiaHepMCVerbosity = cms.untracked.bool(False),
                         crossSection = cms.untracked.double(0),
                         comEnergy = cms.double(5500.0),
                         maxEventsToPrint = cms.untracked.int32(0),
                         PythiaParameters = cms.PSet(
        pythia8CommonSettingsBlock,
        pythia8CP5SettingsBlock,
        processParameters = cms.vstring(
            'Charmonium:states(3S1) = 443', # filter on 443 and prevents other onium states decaying to 443, so we should turn the others off
            'Charmonium:O(3S1)[3S1(1)] = 1.16',
            'Charmonium:O(3S1)[3S1(8)] = 0.0119',
            'Charmonium:O(3S1)[1S0(8)] = 0.01',
            'Charmonium:O(3S1)[3P0(8)] = 0.01',
            'Charmonium:gg2ccbar(3S1)[3S1(1)]g = on',
            'Charmonium:gg2ccbar(3S1)[3S1(8)]g = on',
            'Charmonium:qg2ccbar(3S1)[3S1(8)]q = on',
            'Charmonium:qqbar2ccbar(3S1)[3S1(8)]g = on',
            'Charmonium:gg2ccbar(3S1)[1S0(8)]g = on',
            'Charmonium:qg2ccbar(3S1)[1S0(8)]q = on',
            'Charmonium:qqbar2ccbar(3S1)[1S0(8)]g = on',
            'Charmonium:gg2ccbar(3S1)[3PJ(8)]g = on',
            'Charmonium:qg2ccbar(3S1)[3PJ(8)]q = on',
            'Charmonium:qqbar2ccbar(3S1)[3PJ(8)]g = on',
            'Charmonium:gg2ccbar(3S1)[3S1(1)]gm = on',
            '443:onMode = off',            # ignore cross-section re-weighting (CSAMODE=6) since selecting wanted decay mode
            '443:onIfAny = 13 -13',
            'PhaseSpace:pTHatMin = 2.',
            'PhaseSpace:bias2Selection = on',
            'PhaseSpace:bias2SelectionPow = 1.3',
            'PhaseSpace:bias2SelectionRef = 1'
            ),
        parameterSets = cms.vstring('pythia8CommonSettings',
                                    'pythia8CP5Settings',
                                    'processParameters',
                                    )
        )
                         )

oniafilter = cms.EDFilter("PythiaFilter",
    Status = cms.untracked.int32(2),
    MaxEta = cms.untracked.double(100.0),
    MinEta = cms.untracked.double(-100.0),
    MinPt = cms.untracked.double(0.0),
    ParticleID = cms.untracked.int32(443)
)

mumugenfilter = cms.EDFilter("MCParticlePairFilter",
    Status = cms.untracked.vint32(1, 1),
    MinPt = cms.untracked.vdouble(0., 0.),
    MinP = cms.untracked.vdouble(2.5, 2.5),
    MaxEta = cms.untracked.vdouble(3.5, 3.5),
    MinEta = cms.untracked.vdouble(-3.5, -3.5),
    ParticleCharge = cms.untracked.int32(-1),
    ParticleID1 = cms.untracked.vint32(13),
    ParticleID2 = cms.untracked.vint32(13)
)

ProductionFilterSequence = cms.Sequence(generator*oniafilter*mumugenfilter)
END

)


scram b -j 8
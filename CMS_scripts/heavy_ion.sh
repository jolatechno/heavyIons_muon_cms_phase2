NUM_EVENTS=100

PILEUP_INPUT="dbs:/MTD/phys_heavyions-RECO_Hydjet_Drume5_MinBias_5500GeV_MTD_cmssw1210mtd_06302022v1-ad2d1e0e73320851d31d599182876131/USER instance=prod/phys03"
GEOMETRY=Extended2026D77
ERA=Phase2C11I13M9
NUM_THREADS=4


# GEN-SIM:
echo -e "Generate GEN-SIM script:\n\n"

cmsDriver.py Configuration/GenProduction/python/JpsiMuMu_5p5TeV_Pythia8TuneCP5_cfi.py \
        --fileout file:step1.root --pileup_input "${PILEUP_INPUT}" --python_filename step1.py \
        --mc --eventcontent RAWSIM --pileup HiMixGEN --datatier GEN-SIM \
        --conditions auto:phase2_realistic_T21 --beamspot  MatchHI --step GEN,SIM \
        --nThreads ${NUM_THREADS} --scenario HeavyIons --geometry ${GEOMETRY} --era ${ERA} \
        --no_exec --customise Configuration/DataProcessing/Utils.addMonitoring -n ${NUM_EVENTS}

#echo -e "\n\nRun GEN-SIM script:\n\n"
#
#cmsRun step1.py


# DIGI-RAW:
echo -e "\n\nGenerate DIGI-RAW script:\n\n"

cmsDriver.py step1 --filein file:step1.root --fileout file:step2.root \
        --pileup_input "${PILEUP_INPUT}" \
        --mc --eventcontent FEVTDEBUGHLT --pileup HiMix \
        --datatier GEN-SIM-DIGI-RAW --conditions auto:phase2_realistic_T21 \
        --step DIGI:pdigi_valid,L1,L1TrackTrigger,DIGI2RAW,HLT:@fake2 --nThreads ${NUM_THREADS} \
        --scenario HeavyIons --geometry ${GEOMETRY} --era ${ERA} \
        --python_filename step2.py --no_exec \
        --customise Configuration/DataProcessing/Utils.addMonitoring -n -1

#echo -e "\n\nRun DIGI-RAW script:\n\n"
#
#cmsRun step2.py


# RECO:
echo -e "\n\nGenerate RECO script:\n\n"

cmsDriver.py step2 --filein file:step2.root --fileout file:step3.root \
        --mc --eventcontent MINIAODSIM --runUnscheduled \
        --datatier MINIAODSIM --conditions auto:phase2_realistic_T21 \
        --step RAW2DIGI,RECO,RECOSIM,PAT --nThreads ${NUM_THREADS} \
        --geometry ${GEOMETRY} --era ${ERA} --python_filename step3.py --no_exec \
        --customise Configuration/DataProcessing/Utils.addMonitoring -n -1

#echo -e "\n\nRun RECO script:\n\n"
#
#cmsRun step3.py
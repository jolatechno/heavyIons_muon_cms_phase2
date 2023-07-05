numThreads = 8
memoryPerThread = 2500
filePerJob = 5

from WMCore.Configuration import Configuration

config = Configuration()

config.section_("General")
config.General.requestName = "step4_ONIATREE"
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_("JobType")
config.JobType.pluginName = "Analysis"
config.JobType.psetName = "phase2embeddedMC_oniaAnalyzer.py"
config.JobType.numCores = numThreads
config.JobType.maxMemoryMB = memoryPerThread * numThreads # max authorized = 2500 MB per core
config.JobType.allowUndistributedCMSSW = True

config.section_("Data")
config.Data.inputDataset = '/step1_heavy_ion/jtouzet-step3_RECO-65a7080ce1477c8ffb22deca289a116b/USER'
config.Data.inputDBS = 'phys03'
config.Data.unitsPerJob = filePerJob
config.Data.totalUnits = -1
config.Data.splitting = "FileBased" #"Automatic"
config.Data.allowNonValidInputDataset = True

config.Data.outLFNDirBase = '/store/user/jtouzet/ntuples/%s' % (config.General.requestName)
config.Data.publication = False

config.section_("Site")
config.Site.storageSite = "T2_FR_GRIF_LLR"
config.Site.whitelist = ["T2_FR_GRIF_LLR", "T2_FR_IPHC", "T2_CH_CERN"] # ["T2_FR_*", "T2_CH_*"]
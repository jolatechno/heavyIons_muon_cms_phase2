numThreads = 4
memoryPerThread = 2500
filePerJob = 5

from WMCore.Configuration import Configuration
#from CRABClient.Configuration import Configuration
config = Configuration()

config.section_('General')
config.General.requestName = 'step2_DIGI_RAW'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'PrivateMC'
config.JobType.psetName = 'step2.py' # name of the script created by the cmsDriver.py command
config.JobType.numCores = numThreads
config.JobType.maxMemoryMB = memoryPerThread * numThreads # max authorized = 2500 MB per core
config.JobType.allowUndistributedCMSSW = True

config.section_('Data')
config.Data.inputDataset = '/step1_heavy_ion/jtouzet-step1_GEN_SIM-a44efcaf9fa79751b2333d3e634c7bf3/USER'
config.Data.inputDBS = 'phys03'
config.Data.outLFNDirBase = '/store/user/jtouzet/' 
config.Data.publication = True
config.Data.outputDatasetTag = config.General.requestName
# file-based analysis
config.JobType.pluginName = 'Analysis'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = filePerJob # number of files per job
config.Data.totalUnits = -1

config.section_('Site')
config.Data.ignoreLocality = True
config.Site.storageSite = "T2_FR_GRIF_LLR"
config.Site.whitelist = ["T2_FR_GRIF_LLR", "T2_FR_IPHC", "T2_CH_CERN"] # ["T2_FR_*", "T2_CH_*"]
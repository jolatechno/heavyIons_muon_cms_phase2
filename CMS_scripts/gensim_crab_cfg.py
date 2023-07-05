numThreads = 1
unitsPerJob = 200
memoryPerThread = 2500
numJobs = 10000

from WMCore.Configuration import Configuration
#from CRABClient.Configuration import Configuration
config = Configuration()

config.section_('General')
config.General.requestName = 'step1_GEN_SIM'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'PrivateMC'
config.JobType.psetName = 'step1.py' # name of the script created by the cmsDriver.py command
config.JobType.numCores = numThreads
config.JobType.maxMemoryMB = memoryPerThread * numThreads # max authorized = 2500 MB per core
config.JobType.allowUndistributedCMSSW = True

config.section_('Data')
config.Data.outputPrimaryDataset = 'step1_heavy_ion'
config.Data.splitting = 'EventBased'
config.Data.unitsPerJob = unitsPerJob # number of events per job (mind the filter efficiency!!)
config.Data.totalUnits = config.Data.unitsPerJob * numJobs # total number of generated events
config.Data.outLFNDirBase = '/store/user/jtouzet/' 
config.Data.publication = True
config.Data.outputDatasetTag = config.General.requestName

config.section_('Site')
config.Data.ignoreLocality = True
config.Site.storageSite = "T2_FR_GRIF_LLR"
config.Site.whitelist = ["T2_FR_GRIF_LLR", "T2_FR_IPHC", "T2_CH_CERN"] # ["T2_FR_*", "T2_CH_*"]
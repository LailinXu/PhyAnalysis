#OUTPUT="test_v0"
#INPUT="/afs/cern.ch/user/l/lkaplan/public/cxaod/user.lkaplan.mc14_13TeV.169053.PowhegPythia8_AU2CT10_ggH600NWA_ZZllqq.CAOD_HIGG2D4.v00-00-07-1_outputLabel.root/user.lkaplan.5383028._000001.outputLabel.root"

#INPUT="Runlist/user.lkaplan.mc14_13TeV.169053.PowhegPythia8_AU2CT10_ggH600NWA_ZZllqq.CAOD_HIGG2D4.v00-00-07-1_outputLabel.list"
if [ $# -ge 1 ]; then 
  INPUT=$1
else 
  #INPUT="/gpfs/mnt/gpfs02/usatlas/bnl_local/lailinxu/Analysis/CommonTools/xAODTruth/Runlist/List_p2375/mc15_13TeV.361372.Sherpa_CT10_Zee_Pt0_70_CVetoBVeto.merge.DAOD_HIGG2D4.e3651_s2586_s2174_r6793_r6264_p2375.list"
  INPUT="/gpfs/mnt/gpfs02/usatlas/bnl_local/lailinxu/Analysis/CommonTools/xAODTruth/Runlist/List_p2375/mc15_13TeV.341339.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH1000NW_ZZllqq.merge.DAOD_HIGG2D4.e3940_s2608_s2183_r6630_r6264_p2361.list"
  #INPUT="/usatlas/groups/bnl_local/lailinxu/Analysis/CommonTools/xAODTruth/Runlist/List_p2375/user.lxu.mc15_13TeV.301390.MadGraphPythia8EvtGen_A14NNPDF23LO_HVT_Agv1_VcWZ_llqq_m2000.DAOD_EXOT12.v3.p2375_EXT0.34943123p2375.list"
fi

if [ ! -f ${INPUT} ]; then eosmount ~/eos; fi

outdir=`basename ${INPUT}`
OUTPUT="out_testllqq/${outdir}"
## remove mjj, detajj cut
#OUTPUT="Result/${outdir}"

if [ ! -d `dirname ${OUTPUT}` ]; then
  mkdir `dirname ${OUTPUT}` -p
fi

echo "testllqq -outdir $OUTPUT -in $INPUT"
testllqq -outdir $OUTPUT -in $INPUT

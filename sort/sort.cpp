#include "sort.h"

sort::sort(TString rawfilepathh, TString outfilepathh, TString filenamee, int runnumberstartt, int runnumberstopp)
{
  rawfilepath = rawfilepathh;
  outfilepath = outfilepathh;
  filename = filenamee;
  runnumberstart = runnumberstartt;
  runnumberstop = runnumberstopp;

  benchmark = new TBenchmark;

  count2d = 0;
  count3d = 0;
  fire2cry_total = 0;
  fire2cry_normal = 0;
  fire2cry_dia = 0;
  fire2cry_sep = 0;
  fire3cry_total = 0;
  fire3cry_con1 = 0;
  fire3cry_con2 = 0;
  fire3cry_con3 = 0;
  fire3cry_con4 = 0;
  fire4cry_total = 0;
  memset(energy, 0, 4*NPAR);

  char filenametemp1[1024];
  char tabpath[1024];

  sprintf(filenametemp1, "../cub/mat%04d_%04d.m4b", runnumberstart, runnumberstop);
  cout << "write " << filenametemp1 << "matrix" << endl;
  mat = new Matrix(filenametemp1, EMAX);

  sprintf(filenametemp1, "../cub/cub%04d_%04d", runnumberstart, runnumberstop);
  char tabfile[1024] = TABFILE;
  sprintf(tabpath, "../cub/%s", tabfile);
  cub = new Cube(filenametemp1, tabpath, EMAX);

  sprintf(filenametemp1, "../cub/mat%04d_%04d.root", runnumberstart, runnumberstop);
  fileout = new TFile(filenametemp1, "RECREATE");
  if(!fileout){
    cout << "!!! can not generate " << filenametemp1 << endl;
  }
  else{
    cout << "generate " << filenametemp1 << endl;
  }

  treeout = new TTree("treeout","iThemba sort data");
  treeout->Branch("multi",&multi,"multi/S");
  treeout->Branch("ch",ch,"ch[multi]/S");
  treeout->Branch("clover",clover,"clover[multi]/S");
  treeout->Branch("crystal",crystal,"crystal[multi]/S");
  treeout->Branch("adc",adc,"adc[multi]/s");
  treeout->Branch("timestamp",timestamp,"timestamp[multi]/L");

  treeout->Branch("multi_ab",&multi_ab,"multi_ab/S");
  treeout->Branch("clover_ab",clover_ab,"clover_ab[multi_ab]/S");
  treeout->Branch("energy_ab",energy_ab,"energy_ab[multi_ab]/F");
  treeout->Branch("timestamp_ab",timestamp_ab,"timestamp_ab[multi_ab]/L");

  treeout->Branch("multi_ab_coin",&multi_ab_coin,"multi_ab_coin/S");
  treeout->Branch("clover_ab_coin",clover_ab_coin,"clover_ab_coin[multi_ab_coin]/S");
  treeout->Branch("energy_ab_coin",energy_ab_coin,"energy_ab_coin[multi_ab_coin]/F");

  h2all = new TH2F("h2all", "all vs. all no addback", MATBINNUM, EMIN, EMAX, MATBINNUM, EMIN, EMAX);
  h2all_ab = new TH2F("h2all_ab", "all vs. all with addback", MATBINNUM, EMIN, EMAX, MATBINNUM, EMIN, EMAX);
  h2ado_fd = new TH2F("h2ado_fd", "forward vs. all", MATBINNUM, EMIN, EMAX, MATBINNUM, EMIN, EMAX);
  h2ado_90 = new TH2F("h2ado_90", "90 vs. all", MATBINNUM, EMIN, EMAX, MATBINNUM, EMIN, EMAX);
  h2pol_tran = new TH2F("h2pol_tran", "pol tran", MATBINNUM, EMIN, EMAX, MATBINNUM, EMIN, EMAX);
  h2pol_vert = new TH2F("h2pol_vert", "pol vert", MATBINNUM, EMIN, EMAX, MATBINNUM, EMIN, EMAX);
}

sort::~sort()
{
    delete filein;
    delete fileout;
    delete benchmark;
}

bool sort::openfile(int num)
{
  for(int i=0;i<4*NPAR;i++){
    p0[i] = 0.;
    p1[i] = 0.;
    p2[i] = 0.;
  }
  if(!getcalipar(num)){
    return false;
  }

  char filenametemp2[1024];

  sprintf(filenametemp2, "%s%s%04d.root", rawfilepath.Data(), filename.Data(), num);
  cout << "process " << filenametemp2 << endl;
  filein = new TFile(filenametemp2, "READ");
  if(!filein){
    cout << "cannot open " << filenametemp2 << endl;
    return false;
  }

  treein = (TTree*)filein->Get("tree");

  treein->SetBranchAddress("multi", &multi, &b_multi);
  treein->SetBranchAddress("ch", ch, &b_ch);
  treein->SetBranchAddress("clover", clover, &b_clover);
  treein->SetBranchAddress("crystal", crystal, &b_crystal);
  treein->SetBranchAddress("adc", adc, &b_adc);
  treein->SetBranchAddress("timestamp", timestamp, &b_timestamp);

  totalentry = treein->GetEntries();

  cout << "totalentry: " << totalentry << endl;
  return true;
}

void sort::process()
{
  benchmark->Start("sort ...");
  for(int i=runnumberstart;i<=runnumberstop;i++){
    lep_runnum = i;
    if(i >= 39 && i <= 49) continue;
    if(i == 85 || i == 87 || i == 95) continue;

    if(!openfile(i))
      continue;

    cout << "start runnum: " << i << endl;
    //  do
    //  mkmatV1();
    mkmatV2();
    filein->Close();
  }
  mat->Save();
  cub->Save();
  delete mat;
  delete cub;

  fileout->cd();
  treeout->Write();
  h2all_ab->Write();
  fileout->Close();

  benchmark->Show("sort ...");

  cout << "2d counts:       " << count2d/2 << endl;
  cout << "3d counts:       " << count3d << endl;
  cout << "fire2cry_total:  " << fire2cry_total << endl;
  cout << "fire2cry_normal: " << fire2cry_normal << endl;
  cout << "fire2cry_dia:    " << fire2cry_dia << endl;
  cout << "fire2cry_sep:    " << fire2cry_sep << endl;
  cout << "fire3cry_total:  " << fire3cry_total << endl;
  cout << "fire3cry_con1:   " << fire3cry_con1 << endl;
  cout << "fire3cry_con2:   " << fire3cry_con2 << endl;
  cout << "fire3cry_con3:   " << fire3cry_con3 << endl;
  cout << "fire3cry_con4:   " << fire3cry_con4 << endl;
  cout << "fire4cry_total:  " << fire4cry_total << endl;
}

// use the first fired clover as benchmark
void sort::mkmatV1()
{
  int percent = 0;
  for(int ientry=0;ientry<totalentry;ientry++){
  //  for(int ientry=0;ientry<150000;ientry++){
    if(ientry>=0.1*percent*totalentry){
      cout << 10.0*percent << "%  " << "finished ..." << endl;
      percent++;
    }

    treein->GetEntry(ientry);
    //  cout << "entry: " << ientry << endl;

    //  addbackV1();
    addbackV2();

    multi_ab_coin = 0;
    for(int i=0;i<NPAR;i++){
      clover_ab_coin[i] = 0;
      energy_ab_coin[i] = 0.;
    }

    if(multi_ab<2){
      //  cout << "only one clover fired in entry " << ientry <<endl;
      //treeout->Fill();
      continue;
    }

    clover_ab_coin[multi_ab_coin] = clover_ab[0];
    energy_ab_coin[multi_ab_coin] = energy_ab[0];
    for(int i=1;i<multi_ab;i++){
      if(TMath::Abs(timestamp_ab[i]-timestamp_ab[0])<=COINWINDOW){
        multi_ab_coin++;
        clover_ab_coin[multi_ab_coin] = clover_ab[i];
        energy_ab_coin[multi_ab_coin] = energy_ab[i];
      }
    }
    multi_ab_coin++;

    // fill
    //treeout->Fill();
    if(multi_ab_coin>=2){
      mat->Fill(multi_ab_coin, energy_ab_coin);
      for(int i=0;i<multi_ab_coin;i++){
        for(int j=0;j<multi_ab_coin;j++){
          if(j==i)
            continue;
          h2all_ab->Fill(energy_ab_coin[i], energy_ab_coin[j]);
          count2d++;
        }
      }
    }
    if(multi_ab_coin>=3){
      count3d++;
      cub->Fill(multi_ab_coin, energy_ab_coin);
    }
  }
}

// traverse all fired clover as benchmark
void sort::mkmatV2()
{
  int percent = 0;
  Bool_t flag = 0;
  for(int ientry=0;ientry<totalentry;ientry++){
  //  for(int ientry=0;ientry<1000;ientry++){
    if(ientry>=0.1*percent*totalentry){
      cout << 10.0*percent << "%  " << "finished ..." << endl;
      percent++;
    }

    treein->GetEntry(ientry);
    //  cout << "entry: " << ientry << endl;

    //  addbackV1();
    addbackV2();
    flag = 0;

    //  discard fire energy <= EMIN
    //  discard fire energy >= EMAT
    //  if do not need, command these code

    Short_t multi_ab_all = multi_ab;
    vector<Short_t> vflag_energy;
    for(int j=0;j<multi_ab_all;j++){
     // cout << lep_runnum << endl;
      if((energy_ab[j]<=EMIN || energy_ab[j]>=EMAX) || (lep_runnum >= 50 && lep_runnum <= 108 && (clover_ab[j] == 10 || clover_ab[j] == 11) && (energy_ab[j] <= EMIN || energy_ab[j] > lep_EMAX) ) ){
        vflag_energy.push_back(1);
        multi_ab--;
      }
      else{
        vflag_energy.push_back(0);
      }
    }

    if(multi_ab<2){
     // treeout->Fill();
      continue;
    }

    //  for example
    //  multi_ab = 4; vflag_energy = {0, 1, 1, 0, 0, 1, 0}
    //  --> index_temp[] = {0, 3, 4, 6}
    //  --> vflag_energy_add[] = {0, 2, 2, 3}
    vector<Short_t> index_temp;
    for(int j=0;j<multi_ab_all;j++){
      if(vflag_energy[j]==0){
        index_temp.push_back(j);
      }
    }
    vector<Short_t> vflag_energy_add;
    Short_t s_temp = 0;
    for(int j=0;j<multi_ab;j++){
      for(int jj=0;jj<index_temp[j];jj++){
        s_temp += vflag_energy[jj];
      }
      vflag_energy_add.push_back(s_temp);
      s_temp = 0;
    }

    /*
    cout << "vflag_energy ..." << endl;
    for(vector<Short_t>::iterator it=vflag_energy.begin();it!=vflag_energy.end();++it){
      cout << *it << "  ";
    }
    cout << endl;

    cout << "index_temp ..." << endl;
    for(vector<Short_t>::iterator it=index_temp.begin();it!=index_temp.end();++it){
      cout << *it << "  ";
    }
    cout << endl;

    cout << "vflag_energy_add ..." << endl;
    for(vector<Short_t>::iterator it=vflag_energy_add.begin();it!=vflag_energy_add.end();++it){
      cout << *it << "  ";
    }
    cout << endl;
    */

    //  cout << "multi_ab:  " << multi_ab << endl;
    for(int j=0;j<multi_ab;j++){
      clover_ab[j] = clover_ab[j+vflag_energy_add[j]];
      timestamp_ab[j] = timestamp_ab[j+vflag_energy_add[j]];
      energy_ab[j] = energy_ab[j+vflag_energy_add[j]];
    }
    for(int j=multi_ab;j<NPAR;j++){
      clover_ab[j] = 0;
      timestamp_ab[j] = 0;
      energy_ab[j] = 0;
    }
    //  end here

    multi_ab_coin = 0;
    for(int i=0;i<NPAR;i++){
      clover_ab_coin[i] = 0;
      energy_ab_coin[i] = 0.;
    }

    vector<Short_t> vmulti;
    Short_t multitmp = 0;
    for(int i=0;i<multi_ab;i++){
      multitmp = 1;
      for(int j=0;j<multi_ab;j++){
        if(i==j){
          continue;
        }
        if(TMath::Abs(timestamp_ab[j]-timestamp_ab[i])<=COINWINDOW){
          multitmp++;
        }
      }
      vmulti.push_back(multitmp);
    }

    /*
    vector<Short_t>::iterator vit;
    for(vit=vmulti.begin();vit<vmulti.end();vit++){
      cout << "multi_ab_coin: " << *vit << "  ";
    }
    cout << endl;
    */

    Short_t multi_max = vmulti[0];
    Short_t multi_max_index = 0;
    for(int j=1;j<multi_ab;j++){
      if(vmulti[j]>multi_max){
        multi_max = vmulti[j];
        multi_max_index = j;
      }
    }
    //  cout << "multi_max_index: " << multi_max_index << endl;
    if(multi_max<2){
      //treeout->Fill();
      continue;
    }

    clover_ab_coin[multi_ab_coin] = clover_ab[multi_max_index];
    energy_ab_coin[multi_ab_coin] = energy_ab[multi_max_index];
    for(int i=0;i<multi_ab;i++){
      if(i==multi_max_index){
        continue;
      }
      if(TMath::Abs(timestamp_ab[i]-timestamp_ab[multi_max_index])<=COINWINDOW){
        multi_ab_coin++;
        clover_ab_coin[multi_ab_coin] = clover_ab[i];
        energy_ab_coin[multi_ab_coin] = energy_ab[i];
      }
    }
    multi_ab_coin++;

    // fill
    treeout->Fill();
    if(multi_ab_coin>=2){
      mat->Fill(multi_ab_coin, energy_ab_coin);
      for(int i=0;i<multi_ab_coin;i++){
        for(int j=0;j<multi_ab_coin;j++){
          if(j==i)
            continue;
          h2all_ab->Fill(energy_ab_coin[i], energy_ab_coin[j]);
          count2d++;
        }
      }
    }
    if(multi_ab_coin>=3){
      count3d++;
      cub->Fill(multi_ab_coin, energy_ab_coin);
    }
  }
}


// the same window with event, use the first fired as benchmarking
// if timediff > COINWINDOW, still saved the other fired in this clover
// if 3/4(number) crystals fired in COINWINDOW, discard this event
void sort::addbackV1()
{
  multi_ab = 0;
  for(int i=0;i<NPAR;i++){
    clover_ab[i] = 0;
    timestamp_ab[i] = 0;
    energy_ab[i] = 0;
  }

  Double_t etmp = 0.;
  Int_t hitt = 0;
  Int_t j = 0;
  for(int i=0;i<multi;){
    hitt = 0;
    clover_ab[multi_ab] = clover[i];
    timestamp_ab[multi_ab] = timestamp[i];
    etmp = cali(ch[i], adc[i]);
    while(1){
      j = i+1;
      if(j>=multi){
        i = j;
        //  cout << "end of event" << endl;
        energy_ab[multi_ab] = etmp;
        break;
      }
      if(clover[j]==clover_ab[multi_ab] && (TMath::Abs(timestamp[j]-timestamp[i])<=COINWINDOW)){
        etmp += cali(ch[j], adc[j]);
        hitt++;
        //  cout << "addback ... " << j << endl;
        //  cout << "etmp: " << etmp << endl;
        i = j;
        hitt++;
      }
      else{
        i = j;
        energy_ab[multi_ab] = etmp;
        multi_ab++;
        break;
      }
    }

    //  discard 3,4 his in one clover
    if(hitt>2){
       multi_ab--;
      //  cout << "hit times :" << hitt << endl;
      //  cout << "timestamp : " << j << "  " << timestamp[j] << endl;
      //  cout << "timestamp : " << i << "  " << timestamp[i] << endl;
    }
  }
  multi_ab++;
  /*
  cout << "multi_ab: " << multi_ab << endl;
  for(int i=0;i<multi_ab;i++){
    cout << "event info:" << i << "  " << clover_ab[i] << "  " << "  " << timestamp_ab[i] << "  " << energy_ab[i] << endl;
  }
  */
}


//  contain every situation
void sort::addbackV2()
{
  multi_ab = 0;
  for(int i=0;i<NPAR;i++){
    clover_ab[i] = 0;
    timestamp_ab[i] = 0;
    energy_ab[i] = 0;
  }

  vector<Short_t> index;
  Short_t count = 1;
  Short_t cloveridtmp = clover[0];
  for(int i=1;i<multi;i++){
    if(clover[i]==cloveridtmp){
      count++;
      continue;
    }
    else{
      index.push_back(count);
      cloveridtmp = clover[i];
      count = 1;
    }
  }
  index.push_back(count);

  vector<Short_t>::iterator it;
  vector<Short_t> indexsum;
  Short_t sum = 0;
  for(it=index.begin();it<index.end();it++){
    sum += *it;
    indexsum.push_back(sum-1);
  }

  /*
  cout << "index     ";
  for(int i=0;i<index.size();i++){
    cout << index[i] << "  ";
  }
  cout << endl;

  cout << "indexsum  ";
  for(int i=0;i<indexsum.size();i++){
    cout << indexsum[i] << "  ";
  }
  cout << endl;
  */

  for(int i=0;i<(int)index.size();i++){
    if(index[i]==1){  //  one crystal hit
      clover_ab[multi_ab] = clover[indexsum[i]];
      timestamp_ab[multi_ab] = timestamp[indexsum[i]];
      energy_ab[multi_ab] = cali(ch[indexsum[i]], adc[indexsum[i]]);
      multi_ab++;
    }
    else if(index[i]==2){ //  two crystal hit
      fire2cry_total++;
      if(crystal[indexsum[i]-1]+crystal[indexsum[i]] == 3){ //0+3/1+2
        fire2cry_dia++;
        //  set as two gammas
        for(int j=0;j<2;j++){
          clover_ab[multi_ab] = clover[indexsum[i]-1+j];
          timestamp_ab[multi_ab] = timestamp[indexsum[i]-1+j];
          energy_ab[multi_ab] = cali(ch[indexsum[i]-1+j], adc[indexsum[i]-1+j]);
          multi_ab++;
        }

        /*
        //  still addback
        clover_ab[multi_ab] = clover[indexsum[i]-1];
        timestamp_ab[multi_ab] = timestamp[indexsum[i]-1];
        energy_ab[multi_ab] = cali(ch[indexsum[i]-1], adc[indexsum[i]-1]) + cali(ch[indexsum[i]], adc[indexsum[i]]);
        multi_ab++;
        */
      }
      else{ //0+1/0+2/1+3/2+3
        if(TMath::Abs(timestamp[indexsum[i]-1]-timestamp[indexsum[i]]) <= COINWINDOW){
          fire2cry_normal++;
          clover_ab[multi_ab] = clover[indexsum[i]-1];
          timestamp_ab[multi_ab] = timestamp[indexsum[i]-1];
          energy_ab[multi_ab] = cali(ch[indexsum[i]-1], adc[indexsum[i]-1]) + cali(ch[indexsum[i]], adc[indexsum[i]]);
          multi_ab++;
        }
        else{ //  > COINWINDOW, set as 2 gammas
          fire2cry_sep++;
          for(int j=0;j<2;j++){
            clover_ab[multi_ab] = clover[indexsum[i]-1+j];
            timestamp_ab[multi_ab] = timestamp[indexsum[i]-1+j];
            energy_ab[multi_ab] = cali(ch[indexsum[i]-1+j], adc[indexsum[i]-1+j]);
            multi_ab++;
          }
        }
      }
    }
    else if(index[i]==3){
      fire3cry_total++;
      //  step 1, sort by timestamp
      map<Long64_t, Short_t> mtimestamp_index;
      map<Long64_t, Short_t>::iterator mit;
      for(int j=0;j<3;j++){
        mtimestamp_index.insert({timestamp[indexsum[i]-2+j], indexsum[i]-2+j});
      }
      /*
      for(mit=mtimestamp_index.begin();mit!=mtimestamp_index.end();mit++){
        cout << "timestamp:  " << mit->first << "  index:  " << mit->second << endl;
      }
      */
      map<Long64_t, Short_t>::iterator mit0 = mtimestamp_index.begin();
      map<Long64_t, Short_t>::iterator mit1 = ++mtimestamp_index.begin();
      map<Long64_t, Short_t>::iterator mit2 = --mtimestamp_index.end();
      /*
      cout << "timestamp:  " << mit0->first << "  index:  " << mit0->second << endl;
      cout << "timestamp:  " << mit1->first << "  index:  " << mit1->second << endl;
      cout << "timestamp:  " << mit2->first << "  index:  " << mit2->second << endl;
      */
      Long64_t timedifftmp[3];
      timedifftmp[0] = mit1->first-mit0->first;
      timedifftmp[1] = mit2->first-mit1->first;
      timedifftmp[2] = mit2->first-mit0->first;
      /*
      for(int j=0;j<3;j++){
        cout << "timediff " << j << " :  " << timedifftmp[j] << endl;
      }
      */

      if(timedifftmp[2] <= COINWINDOW){ //  3 crystal fire in COINWINDOW
        fire3cry_con1++;
        //  still addback, command if discard these events
        /*
        clover_ab[multi_ab] = clover[indexsum[i]-2];
        timestamp_ab[multi_ab] = timestamp[indexsum[i]-2];
        energy_ab[multi_ab] = cali(ch[indexsum[i]-2], adc[indexsum[i]-2]) + cali(ch[indexsum[i]-1], adc[indexsum[i]-1]) + cali(ch[indexsum[i]], adc[indexsum[i]]);
        multi_ab++;
        */
      }
      else if(timedifftmp[0]<=COINWINDOW && timedifftmp[1]<=COINWINDOW && timedifftmp[2] > COINWINDOW){  //  0+1 < COINWINDOW, 1+2 < COINWINDOW, 0+2 > COINWINDOW
        fire3cry_con2++;
      }
      else if(timedifftmp[0]>COINWINDOW && timedifftmp[1]>COINWINDOW){  //  3 crystal fire separation
        fire3cry_con3++;
        //  command if discard these events
        for(int j=0;j<3;j++){
          clover_ab[multi_ab] = clover[indexsum[i]-2+j];
          timestamp_ab[multi_ab] = timestamp[indexsum[i]-2+j];
          energy_ab[multi_ab] = cali(ch[indexsum[i]-2+j], adc[indexsum[i]-2+j]);
          multi_ab++;
        }
      }
      else{
        fire3cry_con4++;
        //  command if discard these events
        if(timedifftmp[0]>COINWINDOW && timedifftmp[1]<=COINWINDOW){
          if((crystal[mit1->second]+crystal[mit2->second]) == 3){ //  in COINWINDOW but diag
            for(int j=0;j<3;j++){
              clover_ab[multi_ab] = clover[indexsum[i]-2+j];
              timestamp_ab[multi_ab] = timestamp[indexsum[i]-2+j];
              energy_ab[multi_ab] = cali(ch[indexsum[i]-2+j], adc[indexsum[i]-2+j]);
              multi_ab++;
            }
          }
          else{ //  in COINWINDOW and not diag
            clover_ab[multi_ab] = clover[mit0->second];
            timestamp_ab[multi_ab] = timestamp[mit0->second];
            energy_ab[multi_ab] = cali(ch[mit0->second], adc[mit0->second]);
            multi_ab++;

            clover_ab[multi_ab] = clover[mit1->second];
            timestamp_ab[multi_ab] = timestamp[mit1->second];
            energy_ab[multi_ab] = cali(ch[mit1->second], adc[mit1->second]) + cali(ch[mit2->second], adc[mit2->second]);
            multi_ab++;
          }
        }
        else{
          if((crystal[mit0->second]+crystal[mit1->second]) == 3){ //  in COINWINDOW but diag
            for(int j=0;j<3;j++){
              clover_ab[multi_ab] = clover[indexsum[i]-2+j];
              timestamp_ab[multi_ab] = timestamp[indexsum[i]-2+j];
              energy_ab[multi_ab] = cali(ch[indexsum[i]-2+j], adc[indexsum[i]-2+j]);
              multi_ab++;
            }
          }
          else{ //  in COINWINDOW and not diag
            clover_ab[multi_ab] = clover[mit0->second];
            timestamp_ab[multi_ab] = timestamp[mit0->second];
            energy_ab[multi_ab] = cali(ch[mit0->second], adc[mit0->second]) + cali(ch[mit1->second], adc[mit1->second]);
            multi_ab++;

            clover_ab[multi_ab] = clover[mit2->second];
            timestamp_ab[multi_ab] = timestamp[mit2->second];
            energy_ab[multi_ab] = cali(ch[mit2->second], adc[mit2->second]);
            multi_ab++;
          }
        }
      }
    }
    else{
      fire4cry_total++;
    }
  }

  /*
  cout << "multi_ab: " << multi_ab << endl;
  for(int i=0;i<multi_ab;i++){
    cout << "event info:" << i << "  " << clover_ab[i] << "  " << "  " << timestamp_ab[i] << "  " << energy_ab[i] << endl;
  }
  */
}

bool sort::getcalipar(int runnumberr)
{
  char filenametemp3[1024];

  sprintf(filenametemp3, "%s/R%dener.cal", CALIFILEPATH, runnumberr);
  std::ifstream fcali;
  fcali.open(filenametemp3, std::ifstream::in);
  if(!fcali){
    cout << "can not find " << filenametemp3 << " file" << endl;
    return false;
  }

  double kk[4];
  for(int i=0;i<4*NPAR;i++){
    fcali >> kk[0] >> p0[i] >> p1[i] >> p2[i];
    if(!fcali.good())
      break;
  }

  /*
  cout << "check cali pars" << endl;
  for(int i=0;i<4*NPAR;i++){
    cout << p0[i] << " " << p1[i] << " " << p2[i] << endl;
  }
  */

  return true;
}

Double_t sort::cali(Short_t chh, UShort_t adcc)
{
  Double_t etmp = p0[chh] + adcc*p1[chh] + adcc*adcc*p2[chh];
  //  Double_t etmp = (Double_t)adcc;

  return etmp;
}

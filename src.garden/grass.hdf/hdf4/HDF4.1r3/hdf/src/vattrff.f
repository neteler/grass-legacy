C***********************************************************
C * NCSA HDF
C   *
C * Software Development Group
C   *
C * National Center for Supercomputing Applications
C   *
C * University of Illinois at Urbana-Champaign
C   *
C * 605 E. Springfield, Champaign IL 61820
C   *
C *
C   *
C * For conditions of distribution and use, see the accompanying
C   *
C * hdf/COPYING file.
C   *
C *
C   *
C***************************************************
C
C $Id$
C
C **************************************************
C *
C * vattrff.f
C * Vset attribute Fortran routines
C *
C *************************************************

C --------------- vsffidx ------------------------
C    vsffidx -- find index of a named field in a vdata
C    VSfindex -- vsfcfdx -- vsffidx
C
       integer function vsffidx(vsid, fldnm, fldidx) 
       integer vsid
       character*(*) fldnm
       integer fldidx
       integer vsfcfdx

       vsffidx = vsfcfdx(vsid, fldnm, fldidx, 
     +                  len(fldnm))
       end
C --------------- vsfsnat -------------------------
C    vsfsnat -- set a numeric attribute for a vdata 
C              or a field of a vdata
C    VSsetattr -- vsfcsat -- vsfsnat
C
       integer function vsfsnat(vsid, findex,attrnm,dtype,
     +                        count, values)
       integer vsid, findex, dtype, count
       character*(*) attrnm
       integer  values
       integer vsfcsat
     
       vsfsnat = vsfcsat(vsid, findex, attrnm, dtype,
     +                count, values, len(attrnm))
       end
C --------------- vsfscat -------------------------
C    vsfscat -- set a char type attribute for a vdata 
C              or a field of a vdata
C    VSsetattr -- vsfcsca -- vsfscat
C
       integer function vsfscat(vsid, findex,attrnm,dtype,
     +                      count, values)
       integer vsid, findex, dtype, count
       character*(*) attrnm
       character*(*) values
       integer vsfcsca
     
       vsfscat = vsfcsca(vsid, findex, attrnm, dtype,
     +         count, values, len(attrnm))
       end
C --------------- vsffdat -------------------------
C    vsffdat -- find attribute of a vdata of a field
C               of the vdata
C    VSfindattr -- vsfcfda -- vsffdat
C
       integer function vsffdat(vsid, findex, attrnm)
       integer vsid, findex
       character*(*) attrnm
       integer vsfcfda

       vsffdat = vsfcfda(vsid, findex, attrnm, 
     +                   len(attrnm))
       end 
C ------------------- vsfainf -------------------------
C    vsfainf -- get attribute info
C    VSattrinfo -- vsfcain -- vsfainf
C
       integer function vsfainf(vsid, findex, aindex, attrname,
     +         dtype, count, size)
       integer vsid, findex, aindex, dtype, count, size
       character*(*) attrname
       integer vsfcain

       vsfainf = vsfcain(vsid, findex, aindex, attrname,
     +                   dtype, count, size, len(attrname))
       end 
C ------------------- vfsnatt -------------------------
C    vfsnatt -- set a numeric attr for a vgroup
C    Vsetattr -- vfcsatt -- vfsnatt 
C
       integer function vfsnatt(vgid, attrnm, dtype, 
     +                        count, values)
       integer vgid, dtype, count, values
       character*(*) attrnm
       integer vfcsatt

       vfsnatt = vfcsatt(vgid, attrnm, dtype,count,
     +                  values, len(attrnm))
       end
C -------------- vfscatt -------------------------
C    vfscatt -- set a char attr for a vgroup
C    Vsetattr -- vfcscat -- vfscatt
C
       integer function vfscatt(vgid, attrnm, dtype,
     +                        count, values)
       integer vgid, dtype, count
       character*(*) attrnm, values
       integer vfcscat

       vfscatt = vfcscat(vgid, attrnm, dtype,count,
     +                  values, len(attrnm))
       end
C --------------- vffdatt ---------------------
C    vffdatt -- find an attribute of a vgroup
C    Vfindattr -- vfcfdat -- vffdatt
C
       integer function vffdatt(vg, attrnm)
       integer vg
       character*(*)   attrnm
       integer vfcfdat

       vffdatt = vfcfdat(vg, attrnm, len(attrnm))
       end
C --------------------------------------------

 

     

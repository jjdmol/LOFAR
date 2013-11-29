module gsm_matcher
! This is a module to match two datasets.
! compile with
!	f2py --opt=-O3 -c -m gsm_matcher gsm_matcher.f90 
!or (for OpenMP usage)
!	f2py --f90flags="-fopenmp -lgomp" -lgomp --opt=-O3 -c -m gsm_matcher gsm_matcher.f90 

implicit none

integer, parameter :: MAX_N = 20000
real*8 coord(6, MAX_N, 2), coord_err(3, MAX_N, 2) ! (ra, decl, x, y, z), (ra_err, decl_err, g_major)
integer ids(MAX_N, 2)
integer source_kind(MAX_N, 2) ! 0=point, 1=extended+band, 2=extended+cross-band
integer counts(2)
integer counts_ext(2)
real*8, save :: MAX_MATCH_DISTANCE ! = 0.05d0
real*8, save :: MAX_MATCH_DERUITER ! = 1.0d0
real*8, save :: MAX_MATCH_EXTENDED ! = 0.5d0

contains

subroutine set_params(p1, p2, p3)
real*8 p1, p2, p3
    MAX_MATCH_DISTANCE = p1
    MAX_MATCH_DERUITER = p2
    MAX_MATCH_EXTENDED = p3
end subroutine set_params

! Store data from SQL
subroutine load_data(iid, ncount, iids, source_kinds, values) 
integer, intent(in) :: iid, ncount
integer, intent(in) :: iids(1:ncount), source_kinds(1:ncount)
real*8, intent(in) :: values(8, 1:ncount) ! ra, decl, x, y, z, ra_err, decl_err, g_major

    if ((iid.ne.1).and.(iid.ne.2)) then
        write(*, *) 'Error in gsm_matcher: iid sould be 1 or 2, got: ', iid
        stop
    endif
    if ((ncount.lt.1).or.(ncount.gt.MAX_N)) then
        write(*, *) 'Error in gsm_matcher: ncount should be between 1 and ', MAX_N,', got: ', ncount
        stop
    endif
    counts(iid) = ncount
    coord(1:2, 1:ncount, iid) = values(1:2, 1:ncount)
    coord(3:5, 1:ncount, iid) = values(3:5, 1:ncount)
    coord(6, 1:ncount, iid) = (1d0 - values(5, 1:ncount))**2
    coord_err(:, 1:ncount, iid) = values(6:8, 1:ncount)
    ids(1:ncount, iid) = iids(1:ncount)
    source_kind(1:ncount, iid) = source_kinds(1:ncount)
end subroutine load_data

subroutine do_match(iid_from, matches, match_count, match_distance, match_types)
integer, parameter :: MAX_N = 20000
integer, intent(in) :: iid_from
integer, intent(out) :: matches(2, MAX_N), match_count
real*8, intent(out) :: match_distance(2, MAX_N)
integer, intent(out) :: match_types(MAX_N) !0=point-to-point, 1=extended-to-extended(per-band), 2=extended-to-extended(cross-band)

    integer icount, i, j
    real*8 r_der
    logical bMatched

    if ((iid_from.ne.1).and.(iid_from.ne.2)) then
        write(*, *) 'Error in gsm_matcher: iid_from sould be 1 or 2, got: ', iid_from
        stop
    endif
    icount = 0
    !$omp parallel default (shared) private (i, j, r_der, bMatched)
    !$omp do
    do i = 1, counts(iid_from)
        bMatched = .false.
        !First look for the point-source match or
        !extended source match within the band
        do j = 1, counts(3 - iid_from)
            if ((source_kind(i, iid_from) .eq. source_kind(j, 3-iid_from) ) .and. &
               (any(dabs(coord(3:5, i, iid_from) - coord(3:5, j, 3-iid_from)) .le. MAX_MATCH_DISTANCE))) then
                !We are in the box, now calculate the deRuiter distance.
                if (source_kind(i, iid_from).eq.0) then
                    r_der = get_deRuiter(i, j, iid_from)
                else
                    r_der = get_distance_ext(i, j, iid_from)
                endif
                if ( ((source_kind(i, iid_from).eq.0).and.(r_der.le.MAX_MATCH_DERUITER)).or. &
                     ((source_kind(i, iid_from).ge.1).and.(r_der.le.MAX_MATCH_EXTENDED)) )then
                    icount = icount + 1
                    matches(1, icount) = ids(i, iid_from)
                    matches(2, icount) = ids(j, 3-iid_from)
                    match_types(icount) = source_kind(i, iid_from)+1
                    match_distance(1, icount) = r_der
                    match_distance(2, icount) = get_distance(i, j, iid_from)
                    bMatched = .true.
                endif
            endif
        enddo
        !If the extended source was not matched, then look for
        !cross-band source match.
        if ((source_kind(i, iid_from).eq.1).and.(.not.bMatched)) then
            do j = 1, counts(3 - iid_from)
                if ((source_kind(j, 3-iid_from).eq.2) .and. &
                   (any(dabs(coord(3:5, i, iid_from) - coord(3:5, j, 3-iid_from)) .le. MAX_MATCH_DISTANCE))) then
                    r_der = get_distance_ext(i, j, iid_from)
                    if (r_der.le.MAX_MATCH_EXTENDED)then
                        icount = icount + 1
                        matches(1, icount) = ids(i, iid_from)
                        matches(2, icount) = ids(j, 3-iid_from)
                        match_types(icount) = 3
                        match_distance(1, icount) = r_der
                        match_distance(2, icount) = get_distance(i, j, iid_from)
                        bMatched = .true.
                    endif
                endif
            enddo
        endif
    enddo
    !$omp end do
    !$omp end parallel
    match_count = icount
end subroutine do_match

real*8 function dasind(x) ! arcsin in degree
real*8, intent(in) :: x
    dasind = 57.2957795130823208768d0 * dasin(x)
end function

real*8 function get_distance(i1, i2, iid_from) ! Get physical distance (in 3D space)
integer, intent(in) :: i1, i2, iid_from
real*8 r
    r = dsqrt( sum( (coord(3:5, i1, iid_from) - coord(3:5, i2, 3-iid_from))**2 ))
    get_distance = 2.0d0 * dasind(0.5d0*r)
end function get_distance

real*8 function get_distance_ext(i1, i2, iid_from) ! deRuiter distance for the extended sources
integer, intent(in) :: i1, i2, iid_from
    get_distance_ext = get_distance(i1, i2, iid_from) / (coord_err(3, i1, iid_from)**2 + coord_err(3, i2, 3-iid_from)**2)
end function get_distance_ext

real*8 function get_deRuiter_part(i1, i2, iid_from, ipart) ! Get part <ipart> of deRuiter distance between i1 and i2
integer, intent(in) :: i1, i2, iid_from, ipart
    get_deRuiter_part = (coord(ipart, i1, iid_from) - coord(ipart, i2, 3-iid_from))**2 / &
                        (coord_err(ipart, i1, iid_from)**2 + coord_err(ipart, i2, 3-iid_from)**2 )
end function get_deRuiter_part

real*8 function get_deRuiter(i1, i2, iid_from) ! Get deRuiter distance
integer, intent(in) :: i1, i2, iid_from
  get_deRuiter = dsqrt( coord(6, i1, iid_from) * get_deRuiter_part(i1, i2, iid_from, 1)  &
                                                  + get_deRuiter_part(i1, i2, iid_from, 2))
end function get_deRuiter

end module gsm_matcher

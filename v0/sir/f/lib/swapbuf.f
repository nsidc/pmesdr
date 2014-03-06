c
c
	subroutine swapbuf(t,n)
c
c       swaps bytes for integer*2
c
	logical*1 t(1),s
	do i=1,n
		j=(i-1)*2+1
		s=t(j)
		t(j)=t(j+1)
		t(j+1)=s
	end do
	return
	end

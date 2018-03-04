csvname = commandArgs(trailingOnly=TRUE)[1]
xlab = commandArgs(trailingOnly=TRUE)[2]
ylab = commandArgs(trailingOnly=TRUE)[3]
ylim_lower = as.numeric(commandArgs(trailingOnly=TRUE)[4])
ylim_upper = as.numeric(commandArgs(trailingOnly=TRUE)[5])
table <- read.csv(csvname)


png(paste(csvname,".png",sep=""))

matplot(table,xlab=xlab,ylab=ylab,type="l",ylim=c(ylim_lower,ylim_upper))
#vec = 1:length(colnames(table))
#legend("bottomright",colnames(table),lty=vec,col=vec,bty="n")

dev.off()

png(paste(csvname,"average.png",sep=""))
vec = 1:length(colnames(table))
ave <- apply(table,1,mean)
plot(ave, xlab=xlab, ylab=ylab, type="l", ylim=c(ylim_lower,ylim_upper))

dev.off()
